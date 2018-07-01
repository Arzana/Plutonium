#include "Game.h"
#include "Core\Stopwatch.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\Threading\ThreadUtils.h"
#include <glad\glad.h>
#include <glfw3.h>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

bool Plutonium::ComponentSortPredicate(GameComponent *a, GameComponent *b)
{
	return a->place > b->place;
}

Plutonium::Game::Game(const char * name)
	: device(nullptr), cursor(nullptr), keyboard(nullptr),				// Default state for helper objects.
	FixedTimeStep(true), suppressUpdate(false), suppressRender(true),	// Enable fixed time step and disable first render.
	targetElapTimeFocused(0.0166667f), targetElapTimeNoFocus(0.05f),	// Normal: 60 FPS, Out of focus: 20 FPS.
	accumElapTime(0), maxElapTime(5), loadPercentage(-1)				// Set buffer time objects.
{
	/* Set the main thread name (I think game will always be made on the main thread). */
	_CrtSetCurrentThreadName("main");

	/* Get new window and set helper objects. */
	Window *wnd = new Window(name, Vector2(800.0f, 600.0f));
	ASSERT_IF(!wnd->operational, "Could not create new window!", "Window initialization failed!");

	/* Create device handles. */
	device = new GraphicsAdapter(wnd);
	loader = new AssetLoader(wnd);
	cursor = new Cursor(wnd);
	keyboard = new Keyboard(wnd);
}

Plutonium::Game::~Game(void)
{
	/* Release basic helpers. */
	delete_s(loader);
	delete_s(cursor);
	delete_s(keyboard);
	delete_s(device);

	/* Delete all components. */
	for (size_t i = 0; i < components.size(); i++) delete_s(components.at(i));
	components.clear();

	/* Make sure we finalize the logging pipeline last. */
	_CrtFinalizeLog();
}

void Plutonium::Game::SetTargetTimeStep(int value)
{
	/* Sets the target time step. */
	targetElapTimeFocused = 1.0f / value;
	/* If this is lower than the time step when the game is outside focus then we need to lower the out of focus as well. */
	if (targetElapTimeFocused < targetElapTimeNoFocus) targetElapTimeNoFocus = targetElapTimeFocused;
}

void Plutonium::Game::SetLoadPercentage(int value)
{
	/* Make sure we don't call the method too early or late. */
	if (loadPercentage == -1)
	{
		LOG_WAR("Cannot set load percentage at this point!");
		return;
	}

	loadPercentage.store(value);
}

void Plutonium::Game::Run(void)
{
	Window *wnd = device->GetWindow();

	/* Initialize game. */
	LOG("Initializing '%s'.", wnd->title);
	Stopwatch sw = Stopwatch::StartNew();
	DoInitialize();

	/* Load first level. */
	SetLoadPercentage(loadPercentage = 0);
	LoadContent();

	/* Tick loading until the load percentage has been set to 100. */
	prevTime = glfwGetTime();
	while (loadPercentage < 100)
	{
		wnd->Update();
		while (!Tick(wnd->HasFocus(), true));
	}

	LOG_MSG("Finished initializing and loading content for '%s', took %Lf seconds.", wnd->title, sw.Seconds());

	/* Excecute game loop. */
	while (!wnd->Update())
	{
		while (!Tick(wnd->HasFocus(), false));
		cursor->Reset();
	}

	/* Finalize game. */
	LOG("Finalizing '%s'.", wnd->title);
	UnLoadContent();
	DoFinalize();
}

void Plutonium::Game::LoadNew(void)
{
	/* Unload previous level and load a new one. */
	Stopwatch sw = Stopwatch::StartNew();
	UnLoadContent();
	LoadContent();
	LOG("Done loading new level, took %Lf seconds.", sw.Seconds());
}

void Plutonium::Game::AddComponent(GameComponent * component)
{
	/* Add new component and sort components on specified order. */
	components.push_back(component);
	std::sort(components.begin(), components.end(), ComponentSortPredicate);
}

void Plutonium::Game::Exit(void)
{
	/* Request a close from the window and suppress the render to speed up the closing process. */
	device->GetWindow()->Close();
	suppressRender = true;
}

bool Plutonium::Game::Tick(bool focused, bool loading)
{
	/* Update timers. */
	double curTime = glfwGetTime();
	accumElapTime += static_cast<float>(curTime - prevTime);
	prevTime = curTime;

	float targetElapTime = focused ? targetElapTimeFocused : targetElapTimeNoFocus;

	/* If the update rate is fixed and we're below the threshold we halt the threads execution. */
	if (FixedTimeStep && accumElapTime < targetElapTime)
	{
		duration<float> sleepTime = duration<float>(targetElapTime - accumElapTime);
		this_thread::sleep_for(duration_cast<seconds>(sleepTime));
		return false;
	}

	/* Make sure we don't update to much. */
	if (accumElapTime > maxElapTime) accumElapTime = maxElapTime;
	float dt = 0.0f;

	/* Catch up on game updates. */
	if (suppressUpdate) suppressUpdate = false;
	else
	{
		if (FixedTimeStep)
		{
			/* Do updates. */
			for (; accumElapTime >= targetElapTime; accumElapTime -= targetElapTime)
			{
				dt += targetElapTime;
				if (!loading) DoUpdate(targetElapTime);
			}
		}
		else
		{
			/* Do update. */
			dt = accumElapTime;
			accumElapTime = 0.0f;
			if (!loading) DoUpdate(dt);
		}
	}

	/* Do frame render. */
	if (suppressRender) suppressRender = false;
	else if (loading)
	{
		BeginRender();
		RenderLoad(dt, loadPercentage.load());
		EndRender();
	}
	else DoRender(dt);
	return true;
}

void Plutonium::Game::DoInitialize(void)
{
	/* Make sure the render target is set to the window. */
	device->SetRenderTarget(nullptr);
	drawTimer = new Stopwatch();

	/* Initialize game specific components. */
	Initialize();

	/* Make sure all defined components are initialized. */
	for (size_t i = 0; i < components.size(); i++)
	{
		GameComponent *cur = components.at(i);
		cur->Initialize();
		LOG_WAR_IF(!cur->initialized, "Component at place %d, failed to initialize!", cur->place);
	}
}

void Plutonium::Game::DoFinalize(void)
{
	/* Finalize game specific components. */
	Finalize();

	/* Make sure all defined components are finalized. */
	for (size_t i = 0; i < components.size(); i++)
	{
		GameComponent *cur = components.at(i);
		cur->Finalize();
		LOG_WAR_IF(cur->initialized, "Component at place %d failed to finalize!", cur->place);
	}

	/* Make sure the debugging symbols are freed. */
#if defined(_WIN32)
	_CrtFinalizeWinProcess();
#endif
}

void Plutonium::Game::DoUpdate(float dt)
{
	/* Update all defined components. */
	for (size_t i = 0; i < components.size(); i++)
	{
		GameComponent *cmp = components.at(i);
		if (cmp->enabled) cmp->Update(dt);
	}

	/* Update game specific code. */
	Update(dt);
}

void Plutonium::Game::BeginRender(void)
{
	/* Disable alpha blending and enable culling. */
	device->SetAlphaBlendFunction(BlendState::None);
	device->SetDepthTest(DepthState::LessOrEqual);

	/* Set clear color and clear window. */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	device->Clear(ClearTarget::Color | ClearTarget::Depth);

	/* Call game specific code. */
	PreRender();
}

void Plutonium::Game::DoRender(float dt)
{
	drawTimer->Restart();
	BeginRender();

	/* Render game specific code. */
	Render(dt);

	/* Renders the game specific graphics to the game screen. */
	for (size_t i = 0; i < components.size(); i++) components.at(i)->Render(dt);

	EndRender();
	drawTimer->End();
}

void Plutonium::Game::EndRender(void)
{
	/* Swap render targets. */
	glfwSwapBuffers(device->GetWindow()->hndlr);
}
