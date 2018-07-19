#pragma once
#include <GameLogic\DynamicObject.h>
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
public:
	DynamicObject *object;
	PointLight *light;

	Fire(Game *game, Vector3 pos, Color lightColor, float scale, int weight)
		: animationStarted(false)
	{
		object = new DynamicObject(game, "models/Fire/fire.md2", "fire.png", weight, InitFire);
		object->Teleport((pos - Vector3(0.0f, 33.3f, 0.0f)) * scale);
		object->SetScale(scale);
		object->SetOrientation(0.0f, -PI2, 0.0f);
		light = new PointLight(pos * scale, lightColor, 1.0f, 0.14f, 0.07f);
	}

	void Update(float dt)
	{
		if (!animationStarted)
		{
			object->PlayAnimation("stand");
			animationStarted = true;
		}

		object->Update(dt);
	}

	~Fire(void) noexcept
	{
		delete_s(object);
		delete_s(light);
	}

private:
	bool animationStarted;
};