#pragma once
#include <Graphics\GUI\Containers\Menu.h>

using namespace Plutonium;

struct LoadScreen
	: public Menu
{
public:
	LoadScreen(Game *game);

protected:
	Label *lblLoaderState;
	int32 dotCnt;

	virtual void Initialize(void) override;
	virtual void Create(void) override;
	virtual void Update(float dt) override;
};