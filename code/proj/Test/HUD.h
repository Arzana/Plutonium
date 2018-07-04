#pragma once
#include <Graphics\GUI\Containers\Menu.h>
#include <Components\Diagnostics\FpsCounter.h>
#include <Components\Diagnostics\RamCounter.h>
#include <Components\Diagnostics\VRAMCounter.h>

using namespace Plutonium;

struct HUD
	: Menu
{
public:
	FpsCounter *fps;
	RamCounter *ram;
	VRamCounter *vram;

	HUD(Game *game);

protected:
	Label *lblTime, *lblFps, *lblCpuRam, *lblGpuRam, *lblWorldDrawTime;

	virtual void Initialize(void) override;
	virtual void Create(void) override;

private:
	Label* CreateDefaultLabel(float y);
};