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
	SetDefaultFont("fonts/LucidaConsole.ttf", 24.0f);
}

void LoadScreen::Create(void)
{
	lblLoaderState = AddLabel();
	lblLoaderState->SetAutoSize(true);
	lblLoaderState->SetAnchors(Anchors::Center);
	lblLoaderState->SetBackColor(Color::Transparent);
	lblLoaderState->SetTextColor(Color::WhiteSmoke);
	lblLoaderState->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = game->GetLoader()->GetState();
		for (size_t i = 0; i < dotCnt; i++) text += '.';
	}));

	pbLoadingPercentage = AddProgressBar();
	pbLoadingPercentage->SetBackColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
	pbLoadingPercentage->SetAnchors(Anchors::Center, 0.0f, static_cast<float>(lblLoaderState->GetFont()->GetLineSpace()) * 2.0f);
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

	pbLoadingPercentage->SetValue(clamp(game->GetLoadPercentage(), 0.0f, 1.0f));
}