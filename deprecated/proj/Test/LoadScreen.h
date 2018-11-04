#pragma once
#include <Graphics\GUI\Containers\Menu.h>

using namespace Plutonium;

class LoadScreen
	: public Menu
{
public:
	LoadScreen(Game *game);

protected:
	Label *lblLoaderState;
	ProgressBar *pbGlobal, *pbSingle;
	int32 dotCnt;

	virtual void Initialize(void) override;
	virtual void Create(void) override;
	virtual void Update(float dt) override;
};