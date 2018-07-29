#include "GameLogic\StaticObject.h"

Plutonium::StaticObject::StaticObject(Game * game, const char * model, int loadWeight)
	: parent(game), percentage(loadWeight * 0.01f), model(nullptr)
{
	game->GetLoader()->LoadModel(model, Callback<StaticModel>(this, &StaticObject::OnLoadComplete));
}

Plutonium::StaticObject::StaticObject(Game * game, StaticModel * model, int loadWeight)
	: parent(game), percentage(loadWeight * 0.01f), model(model)
{
	game->UpdateLoadPercentage(percentage);
}

Plutonium::StaticObject::~StaticObject(void)
{
	parent->GetLoader()->Unload(model);
}

void Plutonium::StaticObject::OnLoadComplete(const AssetLoader *, StaticModel * result)
{
	model = result;

	const std::vector<StaticModel::Shape> *shapes = result->GetShapes();
	for (size_t i = 0; i < shapes->size(); i++)
	{
		bb = Box::Merge(bb, shapes->at(i).Mesh->GetBoundingBox());
	}

	parent->UpdateLoadPercentage(percentage);
}