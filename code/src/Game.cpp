#include "Game.h"
#include "Core\Stopwatch.h"
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
	: wnd(nullptr), cursor(nullptr), keyboard(nullptr),					// Default state for helper objects.
	FixedTimeStep(true), suppressRender(false),							// Enable fixed time step and allow first render.
	targetElapTimeFocused(0.0166667f), targetElapTimeNoFocus(0.05f),	// Normal: 60 FPS, Out of focus: 20 FPS.
	accumElapTime(0), maxElapTime(5), loadPercentage(-1)				// Set buffer time objects.
{
	/* Get new window and set helper objects. */
	wnd = new Window(name, Vector2(800.0f, 600.0f));
	ASSERT_IF(!wnd->operational, "Could not create new window!", "Window initialization failed!");

	cursor = new Cursor(wnd);
	keyboard = new Keyboard(wnd);
}

Plutonium::Game::~Game(void)
{
	/* Release basic helpers. */
	delete_s(wnd);
	delete_s(cursor);
	delete_s(keyboard);

	/* Delete all components. */
	for (size_t i = 0; i < components.size(); i++)
	{
		GameComponent *cur = components.at(i);
		delete_s(cur);
	}
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

	/* Render loading screen. */
	BeginRender();
	RenderLoad(accumElapTime, loadPercentage = value);
	EndRender();
}

void Plutonium::Game::Run(void)
{
	/* Initialize game. */
	LOG("Initializing '%s'.", wnd->title);
	prevTime = glfwGetTime();
	DoInitialize();

	/* Load first level. */
	Stopwatch sw = Stopwatch::StartNew();
	SetLoadPercentage(loadPercentage = 0);
	LoadContent();
	LOG("Finished loading content for '%s', took %Lf seconds.", wnd->title, sw.Seconds());

	/* Excecute game loop. */
	while (!wnd->Update())
	{
		while (!Tick());
	}

	/* Finalize game. */
	LOG("Finalizing '%s'.", wnd->title);
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
	wnd->Close();
	suppressRender = true;
}

bool Plutonium::Game::Tick(void)
{
	/* Update timers. */
	double curTime = glfwGetTime();
	float deltaTime = static_cast<float>(curTime - prevTime);
	float targetElapTime = wnd->HasFocus() ? targetElapTimeFocused : targetElapTimeNoFocus;

	accumElapTime += deltaTime;
	prevTime = curTime;

	/* If the update rate is fixed and we're below the threshold we halt the threads execution. */
	if (FixedTimeStep && accumElapTime < targetElapTime)
	{
		duration<float> sleepTime = duration<float>(targetElapTime - accumElapTime);
		this_thread::sleep_for(duration_cast<seconds>(sleepTime));
		return false;
	}

	/* Catch up on game updates. */
	if (FixedTimeStep)
	{
		/* Make sure we don't update to much. */
		if (accumElapTime > maxElapTime) accumElapTime = maxElapTime;
		/* Make sure the delta time for the render function is still correct. */
		deltaTime = accumElapTime;

		/* Do updates. */
		for (; accumElapTime >= targetElapTime; accumElapTime -= targetElapTime) DoUpdate(deltaTime);
	}
	else
	{
		/* Do update. */
		accumElapTime = 0;
		DoUpdate(deltaTime);
	}

	/* Do frame render. */
	if (suppressRender) suppressRender = false;
	else DoRender(deltaTime);
	return true;
}

void Plutonium::Game::DoInitialize(void)
{
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
		LOG_WAR_IF(cur->initialized, "Camponent at place %d failed to finalize!", cur->place);
	}
}

void Plutonium::Game::DoUpdate(float dt)
{
	/* Update all defined components. */
	for (size_t i = 0; i < components.size(); i++) components.at(i)->Update(dt);
	/* Update game specific code. */
	Update(dt);
}

void Plutonium::Game::BeginRender(void)
{
	/* Set window specific viewport. */
	const Rectangle vp = wnd->GetClientBounds();
	glViewport(static_cast<int>(vp.Position.X), static_cast<int>(vp.Position.Y), static_cast<int>(vp.GetWidth()), static_cast<int>(vp.GetHeight()));

	/* Set clear color and clear window. */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Disable alpha blending and enable culling. */
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void Plutonium::Game::DoRender(float dt)
{
	BeginRender();

	/* Renders the game specific graphics to the game screen. */
	for (size_t i = 0; i < components.size(); i++) components.at(i)->Render(dt);
	Render(dt);

	EndRender();
}

void Plutonium::Game::EndRender(void)
{
	/* Swap render targets. */
	glfwSwapBuffers(wnd->hndlr);
}
