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
	LoadTexture("textures/ui/stone.png");
	LoadTexture("textures/ui/water.png");
}

void LoadScreen::Create(void)
{
	lblLoaderState = AddLabel();
	lblLoaderState->SetAutoSize(true);
	lblLoaderState->SetAnchors(Anchors::TopLeft);
	lblLoaderState->SetBackColor(Color::Transparent());
	lblLoaderState->SetTextColor(Color::WhiteSmoke());
	lblLoaderState->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = game->GetLoader()->GetState();
		for (size_t i = 0; i < dotCnt; i++) text += '.';
	}));

	pbSingle = AddProgressBar();
	pbSingle->SetBackgroundImage(GetTexture("stone"));
	pbSingle->SetBarImage(GetTexture("water"));
	pbSingle->SetBarColor(Color::White());
	pbSingle->SetWidth(600);
	pbSingle->SetAnchors(Anchors::BottomCenter);

	pbGlobal = AddProgressBar();
	pbGlobal->SetBackgroundImage(GetTexture("stone"));
	pbGlobal->SetBarImage(GetTexture("water"));
	pbGlobal->SetBarColor(Color::White());
	pbGlobal->SetWidth(600);
	pbGlobal->SetAnchors(Anchors::BottomCenter, 0.0f, static_cast<float>(-pbSingle->GetHeight()));
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

	pbSingle->SetValue(clamp(game->GetLoader()->GetProgression(), 0.0f, 1.0f));
	pbGlobal->SetValue(clamp(game->GetLoadPercentage(), 0.0f, 1.0f));
}