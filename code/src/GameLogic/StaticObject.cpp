#include "GameLogic\StaticObject.h"

Plutonium::StaticObject::StaticObject(Game * game, const char * model, int loadWeight)
	: parent(game), percentage(loadWeight* 0.01f), model(nullptr)
{
	game->GetLoader()->LoadModel(model, Callback<StaticModel>(this, &StaticObject::OnLoadComplete));
}

Plutonium::StaticObject::~StaticObject(void)
{
	parent->GetLoader()->Unload(model);
}

void Plutonium::StaticObject::OnLoadComplete(const AssetLoader *, StaticModel * result)
{
	model = result;
	parent->UpdateLoadPercentage(percentage);
}