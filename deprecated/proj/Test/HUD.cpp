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
	lblTime->SetTextBind(Label::Binder([&](const Label*, ustring &text)
	{
		text = U"Time: ";
		text += ipart(fmodf(6.0f + map(0.0f, 24.0f, tgame->sunAngle, 0.0f, TAU), 24.0f));

	}));
	y += yAdded;

	lblFps = CreateDefaultLabel(y);
	wndDbg->AddItem(lblFps);
	lblFps->SetTextBind(Label::Binder([&](const Label*, ustring &text)
	{
		text = U"Fps (avrg): ";
		text += ipart(fps->GetAverageHz());
		text += U" Hz";
	}));
	y += yAdded;


	lblCpuRam = CreateDefaultLabel(y);
	wndDbg->AddItem(lblCpuRam);
	lblCpuRam->SetTextBind(Label::Binder([&](const Label*, ustring &text)
	{
		text = U"RAM: ";
		text.AddShortBytes(ram->GetAverage());
		text += U" / ";
		text.AddShortBytes(ram->GetOSBudget());
	}));
	y += yAdded;

	lblGpuRam = CreateDefaultLabel(y);
	wndDbg->AddItem(lblGpuRam);
	lblGpuRam->SetTextBind(Label::Binder([&](const Label*, ustring &text)
	{
		text = U"GPU: ";
		text.AddShortBytes(vram->GetAverage());
		text += U" / ";
		text.AddShortBytes(vram->GetOSBudget());
	}));
	y += yAdded;

	lblWorldDrawTime = CreateDefaultLabel(y);
	wndDbg->AddItem(lblWorldDrawTime);
	lblWorldDrawTime->SetTextBind(Label::Binder([&](const Label*, ustring &text)
	{
		const float ms = static_cast<float>(game->GetGlobalRenderTime());

		text = U"World Draw Time: ";
		text.AddFormattedFloat("%.2f", ms);
		text += U" ms (";
		text.AddFormattedFloat("%.0f", 1.0f / ms * 1000.0f);
		text += U"Hz, ";
		text += _CrtGetDrawCalls();
		text += U"Draw Calls)";
	}));
	y += yAdded;

	btnVsync = CreateDefaultButton(y);
	wndDbg->AddItem(btnVsync);
	btnVsync->SetText(game->GetGraphics()->GetWindow()->GetRetraceMode() == VSyncMode::Enabled ? U"Disable VSync" : U"Enable VSync");
	btnVsync->LeftClicked.Add([&](const Button*, CursorHandler)
	{
		Window *wnd = game->GetGraphics()->GetWindow();

		if (wnd->GetRetraceMode() == VSyncMode::Enabled)
		{
			btnVsync->SetText(U"Enable VSync");
			wnd->SetMode(VSyncMode::Disable);
		}
		else
		{
			btnVsync->SetText(U"Disable VSync");
			wnd->SetMode(VSyncMode::Enabled);
		}
	});

	y += btnVsync->GetSize().Y + OFFSET;

	lblDayNight = CreateDefaultLabel(y);
	wndDbg->AddItem(lblDayNight);
	lblDayNight->SetText(U"Day/Night speed: ");

	sldDayNight = AddSlider();
	wndDbg->AddItem(sldDayNight);
	sldDayNight->SetPosition(Vector2(lblDayNight->GetSize().X + OFFSET, y));
	sldDayNight->SetValueMapped(tgame->dayNightSpeed, 0.0f, 25.0f);
	sldDayNight->ValueChanged.Add([&](const ProgressBar *sender, ValueChangedEventArgs<float>)
	{
		tgame->dayNightSpeed = sender->GetValueMapped(0.0f, 25.0f);
	});

	y += sldDayNight->GetSize().Y + OFFSET;

	lblExposure = CreateDefaultLabel(y);
	wndDbg->AddItem(lblExposure);
	lblExposure->SetText(U"Exposure: ");

	sldExposure = AddSlider();
	wndDbg->AddItem(sldExposure);
	sldExposure->SetPosition(Vector2(lblDayNight->GetSize().X, y));
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

Button * HUD::CreateDefaultButton(float y)
{
	Button *result = AddButton();

	result->SetAutoSize(true);
	result->SetBackColor(Color::Black() * 0.5f);
	result->SetTextColor(Color::White());
	result->SetPosition(Vector2(0.0f, y));
	result->HoverEnter.Add([&](const GuiItem *sender, CursorHandler) { const_cast<GuiItem*>(sender)->SetBackColor(Color::Abbey() * 0.5f); });
	result->HoverLeave.Add([&](const GuiItem *sender, CursorHandler) { const_cast<GuiItem*>(sender)->SetBackColor(Color::Black() * 0.5f); });

	return result;
}