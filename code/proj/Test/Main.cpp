#include <Game.h>
#include <Components\FpsCounter.h>
#include <Graphics\Text\DebugTextRenderer.h>
#include <Core\String.h>
#include <Graphics\Rendering\Renderer.h>
#include "Components\Camera.h"
#include "Core\Math\Basics.h"

using namespace Plutonium;

struct TestGame
	: public Game
{
	FpsCounter *fps;
	Camera *cam;
	DebugFontRenderer *fontRenderer;
	Renderer *renderer;
	Model *heart, *heart2;
	Vector3 light = Vector3::Zero;

	TestGame(void)
		: Game("TestGame")
	{}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		fontRenderer = new DebugFontRenderer(GetWindow(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Debug_Text.vsh", "./assets/shaders/Debug_Text.fsh");
		renderer = new Renderer("./assets/shaders/Basic3D.vsh", "./assets/shaders/Basic3D.fsh");
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetWindow());
		heart = Model::FromFile("./assets/models/Heart/Heart.obj");
		heart2 = Model::FromFile("./assets/models/Heart/Heart.obj");
		heart2->SetScale(10.0f);
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(heart);
		delete_s(heart2);
	}

	virtual void Finalize(void)
	{
		delete_s(fontRenderer);
		delete_s(renderer);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		static float theta = 0.0f;
		theta = modrads(theta += DEG2RAD * dt * 100);
		light = Vector3(cosf(theta), 0.0f, sinf(theta));
		String lightStr = "Light ";
		fontRenderer->AddDebugString((lightStr += ipart(theta * RAD2DEG)) += "°");

		/* Update camera. */
		cam->Update(dt, heart->GetWorld());

		/* Update input. */
		KeyHandler keyboard = GetKeyboard();
		if (keyboard->IsKeyDown(Keys::W)) heart->Move(Vector3::Forward * dt);
		if (keyboard->IsKeyDown(Keys::A)) heart->Move(Vector3::Left * dt);
		if (keyboard->IsKeyDown(Keys::S)) heart->Move(Vector3::Backward * dt);
		if (keyboard->IsKeyDown(Keys::D)) heart->Move(Vector3::Right * dt);
	}

	virtual void Render(float dt)
	{
		/* Render current UPS. */
		String upsStr = "Ups (cur): ";
		fontRenderer->AddDebugString(upsStr += ipart(fps->GetUps()));

		/* Render current FPS. */
		String fpscStr = "Fps (cur): ";
		fontRenderer->AddDebugString(fpscStr += ipart(fps->GetCurFps()));

		/* Render average FPS. */
		String fpsaStr = "Fps (avg): ";
		fontRenderer->AddDebugString(fpsaStr += ipart(fps->GetAvrgFps()));

		/* Render models. */
		renderer->Begin(cam->GetView(), cam->GetProjection(), light);
		renderer->Render(heart);
		renderer->Render(heart2);
		renderer->End();

		/* Render text. */
		fontRenderer->Render();
	}

	virtual void RenderLoad(float dt, int percentage)
	{}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

	return 0;
}