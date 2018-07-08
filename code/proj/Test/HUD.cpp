#include "HUD.h"
#include "TestGame.h"
#include <Core\Math\Interpolation.h>
#include <Core\String.h>

HUD::HUD(Game *game)
	: Menu(game)
{
	Hide();
}

void HUD::Initialize(void)
{
	Menu::Initialize();

	SetDefaultFont("fonts/OpenSans-Regular.ttf", 24.0f);
	LoadTexture("textures/water.png");
	LoadTexture("textures/stone.png");
	LoadTexture("textures/uv.png");

	game->AddComponent(fps = new FpsCounter(game));
	game->AddComponent(ram = new RamCounter(game));
	game->AddComponent(vram = new VRamCounter(game));
}

void HUD::Create(void)
{
	float y = 0.0f;
	float yAdded = static_cast<float>(GetDefaultFont()->GetLineSpace());

	lblTime = CreateDefaultLabel(y);
	lblTime->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		const TestGame *tgame = dynamic_cast<TestGame*>(game);
		text = "Time: ";
		text += tgame->dayState;
		text += ' ';
		text += std::to_string(ipart(fmodf(6.0f + map(0.0f, 24.0f, tgame->sunAngle, 0.0f, TAU), 24.0f)));

	}));
	y += yAdded;

	lblFps = CreateDefaultLabel(y);
	lblFps->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "Fps (avrg): ";
		text += std::to_string(ipart(fps->GetAverageHz()));
		text += " Hz";
	}));
	y += yAdded;

	lblCpuRam = CreateDefaultLabel(y);
	lblCpuRam->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "RAM: ";
		text += b2short_string(ram->GetAverage());
		text += " / ";
		text += b2short_string(ram->GetOSBudget());
	}));
	y += yAdded;

	lblGpuRam = CreateDefaultLabel(y);
	lblGpuRam->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "GPU: ";
		text += b2short_string(vram->GetAverage());
		text += " / ";
		text += b2short_string(vram->GetOSBudget());
	}));
	y += yAdded;

	lblWorldDrawTime = CreateDefaultLabel(y);
	lblWorldDrawTime->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "World Draw Time: ";
		text += to_string("%.2f", static_cast<float>(game->GetGlobalRenderTime()));
		text += " ms";
	}));

	Slider *sld = AddSlider();
	sld->SetRoundingFactor(0.0f);
	sld->SetAnchors(_CrtEnumBitOr(Anchors::Top, Anchors::CenterWidth), 0.0f, 15.0f);
	sld->SetValueMapped(dynamic_cast<TestGame*>(game)->cam->GetFov(), 0.0f, PI2);
	sld->ValueChanged.Add([&](const ProgressBar*, ValueChangedEventArgs<float> args) 
	{
		dynamic_cast<TestGame*>(game)->cam->SetFoV(lerp(0.0f, PI2, args.NewValue));
	});
}

Label * HUD::CreateDefaultLabel(float y)
{
	Label *result = AddLabel();

	result->SetAutoSize(true);
	result->SetBackColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
	result->SetTextColor(Color::White);
	result->SetPosition(Vector2(0.0f, y));

	return result;
}