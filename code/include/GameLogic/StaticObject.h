#pragma once
#include "WorldObject.h"
#include "Game.h"

namespace Plutonium
{
	/* Defines a static world object. */
	struct StaticObject
		: WorldObject
	{
	public:
		/* Initializes a new instance of a static object. */
		StaticObject(_In_ Game *game, _In_ const char *model, _In_ int loadWeight);
		/* Initializes a new instance of a static object with a pre-defined model. */
		StaticObject(_In_ Game *game, _In_ StaticModel *model, _In_ int loadWeight);
		StaticObject(_In_ const StaticObject &value) = delete;
		StaticObject(_In_ StaticObject &&value) = delete;
		/* Releases the resources allocated by the object. */
		~StaticObject(void);

		_Check_return_ StaticObject& operator =(_In_ const StaticObject &other) = delete;
		_Check_return_ StaticObject& operator =(_In_ StaticObject &&other) = delete;

		/* Gets the underlying model of the object. */
		_Check_return_ inline const StaticModel* GetModel(void) const
		{
			return model;
		}

	private:
		Game *parent;
		StaticModel *model;
		float percentage;

		void OnLoadComplete(const AssetLoader*, StaticModel *result);
	};
}