#pragma once
#include <Graphics\Models\DynamicModel.h>
#include <Graphics\Lighting\PointLight.h>

using namespace Plutonium;

inline void InitFire(const char *Name, PlayBackFlags &flags, float &fps)
{
	if (!strcmp(Name, "stand"))
	{
		flags = PlayBackFlags::Loop;
		fps = 60;
	}
	else LOG_WAR("Unset Fire animation: %s!", Name);
}

struct Fire
{
	DynamicModel *model;
	PointLight *light;

	Fire(WindowHandler wnd, const char *path, const char *texture, Vector3 pos, float scale)
	{
		model = DynamicModel::FromFile(path, wnd, texture);
		model->Teleport((pos - Vector3(0.0f, 33.3f, 0.0f)) * scale);
		model->SetScale(scale);
		model->SetOrientation(0.0f, -PI2, 0.0f);
		model->Initialize(InitFire);
		model->PlayAnimation("stand");
		light = new PointLight(pos * scale, Color((byte)254, 211, 60), 1.0f, 0.14f, 0.07f);
	}

	void Update(float dt)
	{
		model->Update(dt);
	}
	
	~Fire(void) noexcept 
	{
		delete_s(model);
		delete_s(light);
	}
};