#pragma once
#include <Graphics\Rendering\Deferred\DeferredRendererBP.h>
#include <Graphics\Rendering\SkyboxRenderer.h>
#include <Graphics\Rendering\ShapeRenderer.h>
#include <Graphics\Lighting\DirectionalLight.h>
#include <Components\Camera.h>
#include <Game.h>
#include "Fire.h"
#include "Knight.h"
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
	Knight *knight;
	StaticObject *map;
	TextureHandler skybox;

	/* Display values. */
	float sunAngle, dayNightSpeed;

	/* Renderers. */
	DeferredRendererBP *renderer;
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
	SkyboxRenderer *sbrenderer;
	ShapeRenderer *srenderer;
	LoadScreen *loadScreen;
	HUD *hud;

	void UpdateDayState(float dt);
	void SpecialKeyPress(WindowHandler, const KeyEventArgs args);
};