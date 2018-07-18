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

	SetDefaultFont("fonts/OpenSans-Regular.ttf", 20.0f);
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
	const float yAdded = static_cast<float>(GetDefaultFont()->GetLineSpace());

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

	btnDayNight = AddButton();
	btnDayNight->SetAutoSize(true);
	btnDayNight->SetBackColor(Color::Black() * 0.5f);
	btnDayNight->SetTextColor(Color::White());
	btnDayNight->SetAnchors(Anchors::TopCenter);
	btnDayNight->SetText(dynamic_cast<TestGame*>(game)->enableDayNight ? "Disable Day/Night Cycle" : "Enable Day/Night Cycle");
	btnDayNight->LeftClicked.Add([&](const Button*, CursorHandler)
	{
		TestGame *tgame = dynamic_cast<TestGame*>(game);
		if (tgame->enableDayNight) btnDayNight->SetText("Enable Day/Night Cycle");
		else btnDayNight->SetText("Diable Day/Night Cycle");

		tgame->enableDayNight = !tgame->enableDayNight;
	});

	sldExposure = AddSlider();
	sldExposure->SetAnchors(Anchors::TopCenter, 0.0f, btnDayNight->GetSize().Y * 2.0f);
	sldExposure->SetValueMapped(dynamic_cast<TestGame*>(game)->renderer->Exposure, 0.0f, 10.0f);
	sldExposure->ValueChanged.Add([&](const ProgressBar *sender, ValueChangedEventArgs<float>)
	{
		dynamic_cast<TestGame*>(game)->renderer->Exposure = sender->GetValueMapped(0.0f, 10.0f);
	});
}

Label * HUD::CreateDefaultLabel(float y)
{
	Label *result = AddLabel();

	result->SetAutoSize(true);
	result->SetBackColor(Color::Black() * 0.5f);
	result->SetTextColor(Color::White());
	result->SetPosition(Vector2(0.0f, y));

	return result;
}