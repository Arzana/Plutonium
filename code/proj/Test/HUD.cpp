#include "HUD.h"
#include "TestGame.h"
#include <Core\Math\Interpolation.h>
#include <Core\String.h>
#include <Core\StringFunctions.h>
#include <Graphics\Native\OpenGL.h>

HUD::HUD(Game *game)
	: Menu(game), tgame(dynamic_cast<TestGame*>(game))
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
	constexpr float OFFSET = 2.0f;
	float y = 0.0f;
	const float yAdded = static_cast<float>(GetDefaultFont()->GetLineSpace()) + OFFSET;

	wndDbg = AddWindow();
	wndDbg->SetCloseResponse(true);
	wndDbg->SetAnchors(Anchors::TopLeft);
	wndDbg->SetBackColor(Color::White() * 0.5f);
	wndDbg->SetHeaderColor(Color::Black() * 0.5f);
	wndDbg->SetHeaderTextColor(Color::White());
	wndDbg->SetTitle(U"Debug Window");

	lblTime = CreateDefaultLabel(y);
	wndDbg->AddItem(lblTime);
	lblTime->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "Time: ";
		text += std::to_string(ipart(fmodf(6.0f + map(0.0f, 24.0f, tgame->sunAngle, 0.0f, TAU), 24.0f)));

	}));
	y += yAdded;

	lblFps = CreateDefaultLabel(y);
	wndDbg->AddItem(lblFps);
	lblFps->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "Fps (avrg): ";
		text += std::to_string(ipart(fps->GetAverageHz()));
		text += " Hz";
	}));
	y += yAdded;
	

	lblCpuRam = CreateDefaultLabel(y);
	wndDbg->AddItem(lblCpuRam);
	lblCpuRam->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "RAM: ";
		text += b2short_string(ram->GetAverage());
		text += " / ";
		text += b2short_string(ram->GetOSBudget());
	}));
	y += yAdded;

	lblGpuRam = CreateDefaultLabel(y);
	wndDbg->AddItem(lblGpuRam);
	lblGpuRam->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		text = "GPU: ";
		text += b2short_string(vram->GetAverage());
		text += " / ";
		text += b2short_string(vram->GetOSBudget());
	}));
	y += yAdded;

	lblWorldDrawTime = CreateDefaultLabel(y);
	wndDbg->AddItem(lblWorldDrawTime);
	lblWorldDrawTime->SetTextBind(Label::Binder([&](const Label*, std::string &text)
	{
		const float ms = static_cast<float>(game->GetGlobalRenderTime());

		text = "World Draw Time: ";
		text += to_string("%.2f", ms);
		text += " ms (";
		text += to_string("%.0f", 1.0f / ms * 1000.0f);
		text += "Hz, ";
		text += std::to_string(_CrtGetDrawCalls());
		text += "Draw Calls)";
	}));

	btnDayNight = CreateDefaultButton();
	btnDayNight->SetAnchors(Anchors::TopCenter);
	btnDayNight->SetText(dynamic_cast<TestGame*>(game)->enableDayNight ? "Disable Day/Night Cycle" : "Enable Day/Night Cycle");
	btnDayNight->LeftClicked.Add([&](const Button*, CursorHandler)
	{
		if (tgame->enableDayNight) btnDayNight->SetText("Enable Day/Night Cycle");
		else btnDayNight->SetText("Disable Day/Night Cycle");

		tgame->enableDayNight = !tgame->enableDayNight;
	});

	y = btnDayNight->GetSize().Y + OFFSET;

	btnVsync = CreateDefaultButton();
	btnVsync->SetAnchors(Anchors::TopCenter, 0.0f, y);
	btnVsync->SetText(game->GetGraphics()->GetWindow()->GetRetraceMode() == VSyncMode::Enabled ? "Disable VSync" : "Enable VSync");
	btnVsync->LeftClicked.Add([&](const Button*, CursorHandler)
	{
		Window *wnd = game->GetGraphics()->GetWindow();

		if (wnd->GetRetraceMode() == VSyncMode::Enabled)
		{
			btnVsync->SetText("Enable VSync");
			wnd->SetMode(VSyncMode::Disable);
		}
		else
		{
			btnVsync->SetText("Disable VSync");
			wnd->SetMode(VSyncMode::Enabled);
		}
	});

	y += btnVsync->GetSize().Y + OFFSET;

	sldExposure = AddSlider();
	sldExposure->SetAnchors(Anchors::TopCenter, 0.0f, y);
	sldExposure->SetValueMapped(tgame->renderer->Exposure, 0.0f, 10.0f);
	sldExposure->ValueChanged.Add([&](const ProgressBar *sender, ValueChangedEventArgs<float>)
	{
		tgame->renderer->Exposure = sender->GetValueMapped(0.0f, 10.0f);
	});

	if (tgame->knight)
	{
		txtKnightAnim = AddTextBox();
		txtKnightAnim->SetAutoSize(true);
		txtKnightAnim->SetBackColor(Color::Black() * 0.5f);
		txtKnightAnim->SetTextColor(Color::White());
		txtKnightAnim->SetAnchors(Anchors::BottomCenter);
		txtKnightAnim->SetText("stand");
		txtKnightAnim->Confirmed.Add([&](const TextBox *sender, EventArgs)
		{
			const char *anim = heapstr(sender->GetText());
			if (tgame->knight->object->PlayAnimation(anim)) txtKnightAnim->SetBackColor(Color::Black() * 0.5f);
			else txtKnightAnim->SetBackColor(Color::Yellow() * 0.5f);
			free_s(anim);
		});
	}
}

void HUD::Update(float dt)
{
	Menu::Update(dt);

	if (game->GetKeyboard()->IsKeyDown(Keys::OemTilde))
	{
		wndDbg->SetState(true);
		wndDbg->SetVisibility(true);
	}
}

Label * HUD::CreateDefaultLabel(float y)
{
	Label *result = AddLabel();

	result->SetAutoSize(true);
	result->SetBackColor(Color::Transparent());
	result->SetTextColor(Color::White());
	result->SetPosition(Vector2(0.0f, y));

	return result;
}

Button * HUD::CreateDefaultButton(void)
{
	Button *result = AddButton();

	result->SetAutoSize(true);
	result->SetBackColor(Color::Black() * 0.5f);
	result->SetTextColor(Color::White());
	result->HoverEnter.Add([&](const GuiItem *sender, CursorHandler) { const_cast<GuiItem*>(sender)->SetBackColor(Color::Abbey() * 0.5f); });
	result->HoverLeave.Add([&](const GuiItem *sender, CursorHandler) { const_cast<GuiItem*>(sender)->SetBackColor(Color::Black() * 0.5f); });

	return result;
}