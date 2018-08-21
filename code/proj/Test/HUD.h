#pragma once
#include <Graphics\GUI\Containers\Menu.h>
#include <Components\Diagnostics\FpsCounter.h>
#include <Components\Diagnostics\RamCounter.h>
#include <Components\Diagnostics\VRAMCounter.h>

using namespace Plutonium;

struct TestGame;

struct HUD
	: Menu
{
public:
	FpsCounter *fps;
	RamCounter *ram;
	VRamCounter *vram;

	HUD(Game *game);

protected:
	Label *lblTime, *lblFps, *lblCpuRam, *lblGpuRam, *lblWorldDrawTime, *lblExposure, *lblDayNight;
	Button *btnVsync;
	Slider *sldDayNight, *sldExposure;
	TextBox *txtKnightAnim;
	GUIWindow *wndDbg;

	virtual void Initialize(void) override;
	virtual void Create(void) override;
	virtual void Update(float dt) override;

private:
	TestGame *tgame;

	Label* CreateDefaultLabel(float y);
	Button *CreateDefaultButton(float y);
};