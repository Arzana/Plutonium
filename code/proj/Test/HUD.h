#pragma once
#include <Graphics\GUI\Containers\Menu.h>
#include <Core\Math\Interpolation.h>
#include <Core\String.h>

using namespace Plutonium;

struct HUD
	: Menu
{
public:
	HUD(Game *game)
		: Menu(game),
		dayState("NULL"), sunAngle(0.0f), fpsAvrg(0.0f), ram(0), vram(0), cpuBudget(0), gpuBudget(0)
	{}

	void UpdateDisplayValues(const char *dayState, float sunAngle, float fpsAvrg, uint64 ram, uint64 vram, uint64 osBudget, uint64 framBufferSize)
	{
		this->dayState = dayState;
		this->sunAngle = sunAngle;
		this->fpsAvrg = fpsAvrg;
		this->ram = ram;
		this->vram = vram;
		this->cpuBudget = osBudget;
		this->gpuBudget = framBufferSize;
	}

protected:
	Label *lblTime, *lblFps, *lblCpuRam, *lblGpuRam, *lblWorldDrawTime;

	virtual void Initialize(void) override
	{
		Menu::Initialize();
		SetDefaultFont("fonts/OpenSans-Regular.ttf", 24.0f);
		LoadTexture("textures/water.png");
		LoadTexture("textures/stone.png");
		LoadTexture("textures/uv.png");
	}

	virtual void Create(void) override 
	{
		float y = 0.0f;
		float yAdded = static_cast<float>(GetDefaultFont()->GetLineSpace());

		lblTime = CreateDefaultLabel(y);
		y += yAdded;

		lblFps = CreateDefaultLabel(y);
		y += yAdded;

		lblCpuRam = CreateDefaultLabel(y);
		y += yAdded;

		lblGpuRam = CreateDefaultLabel(y);
		y += yAdded;

		lblWorldDrawTime = CreateDefaultLabel(y);

		Button *btn = AddButton();
		btn->MoveRelative(Anchors::Center);
		btn->SetAutoSize(true);
		btn->SetBackColor(Color::White);
		btn->SetText(U"Test Btn");
		btn->SetBackgroundImage(GetTexture("water"));
		btn->SetHoverImage(GetTexture("stone"));
		btn->SetClickImage(GetTexture("uv"));
		btn->LeftClicked.Add([](const Button*, CursorHandler) { LOG("LEFT"); });
		btn->RightClicked.Add([](const Button*, CursorHandler) { LOG("RIGHT"); });
		btn->DoubleClicked.Add([](const Button*, CursorHandler) { LOG("DOUBLE"); });
	}

	virtual void Update(float dt) override
	{
		Menu::Update(dt);

		/* Update time string. */
		std::string strTime = "Time: ";
		((strTime += dayState) += ' ') += std::to_string(ipart(fmodf(6.0f + map(0.0f, 24.0f, sunAngle, 0.0f, TAU), 24.0f)));
		lblTime->SetText(strTime.c_str());
		lblTime->Update(dt);

		/* Update debug average FPS. */
		std::string strFps = "Fps (avg): ";
		strFps += std::to_string(ipart(fpsAvrg));
		strFps += " Hz";
		lblFps->SetText(strFps.c_str());
		lblFps->Update(dt);

		/* Update debug average RAM. */
		std::string strRam = "RAM: ";
		((strRam += b2short_string(ram)) += " / ") += b2short_string(cpuBudget);
		lblCpuRam->SetText(strRam.c_str());
		lblCpuRam->Update(dt);

		/* Update debug average GRAM. */
		std::string strGpu = "GPU: ";
		((strGpu += b2short_string(vram)) += " / ") += b2short_string(gpuBudget);
		lblGpuRam->SetText(strGpu.c_str());
		lblGpuRam->Update(dt);

		/* Update debug draw time. */
		std::string strDraw = "World Draw Time: ";
		strDraw += to_string("%.2f", static_cast<float>(game->GetGlobalRenderTime()));
		strDraw += " ms";
		lblWorldDrawTime->SetText(strDraw.c_str());
		lblWorldDrawTime->Update(dt);
	}

private:
	const char *dayState;
	float sunAngle, fpsAvrg;
	uint64 ram, vram, cpuBudget, gpuBudget;

	Label* CreateDefaultLabel(float y)
	{
		Label *result = AddLabel();

		result->SetAutoSize(true);
		result->SetBackColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
		result->SetTextColor(Color::White);
		result->SetPosition(Vector2(0.0f, y));

		return result;
	}
};