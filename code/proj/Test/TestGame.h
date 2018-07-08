#pragma once
#include <Game.h>
#include <Graphics\Diagnostics\DebugMeshRenderer.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Graphics\Rendering\DynamicRenderer.h>
#include <Graphics\Rendering\SkyboxRenderer.h>
#include <Components\Camera.h>
#include <Graphics\Lighting\DirectionalLight.h>
#include "Fire.h"
#include "HUD.h"
#include "LoadScreen.h"

using namespace Plutonium;

struct TestGame
	: Game
{
public:
	/* Scene. */
	DirectionalLight *sun;
	std::vector<Fire*> fires;
	StaticObject *map;
	TextureHandler skybox;

	/* Display values. */
	float sunAngle;
	const char *dayState;

	Camera *cam;

	TestGame(void);

protected:
	virtual void Initialize(void);
	virtual void LoadContent(void);
	virtual void UnLoadContent(void);
	virtual void Finalize(void);
	virtual void Update(float dt);
	virtual void Render(float dt);

private:
	/* Renderers. */
	DebugMeshRenderer * dmrenderer;
	StaticRenderer *srenderer;
	DynamicRenderer *drenderer;
	SkyboxRenderer *sbrenderer;
	DebuggableValues renderMode;

	/* Menus */
	LoadScreen *loadScreen;
	HUD *hud;

	void UpdateDayState(float dt);
	void SpecialKeyPress(WindowHandler, const KeyEventArgs args);
};