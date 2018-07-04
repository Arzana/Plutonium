#include "LoadScreen.h"
#include <string>

LoadScreen::LoadScreen(Game * game)
	: Menu(game), dotCnt(0)
{
	ActiveDuringLoad = true;
}

void LoadScreen::Initialize(void)
{
	Menu::Initialize();
	SetDefaultFont("fonts/OpenSans-Regular.ttf", 24.0f);
}

void LoadScreen::Create(void)
{
	lblLoaderState = AddLabel();
	lblLoaderState->SetAutoSize(true);
	lblLoaderState->MoveRelative(Anchors::Center);
	lblLoaderState->SetBackColor(Color::Transparent);
	lblLoaderState->SetTextColor(Color::WhiteSmoke);
	lblLoaderState->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = game->GetLoader()->GetState();
		text += '(';
		text += std::to_string(game->GetLoadPercentage());
		text += "%)";
		for (size_t i = 0; i < dotCnt; i++) text += '.';
	}));
}

void LoadScreen::Update(float dt)
{
	Menu::Update(dt);

	static float accum = 0.0f;
	if ((accum += dt) > 1.0f)
	{
		accum = 0.0f;
		if (++dotCnt > 3) dotCnt = 0;
	}
}