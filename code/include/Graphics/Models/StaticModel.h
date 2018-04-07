#pragma once
#include "GameLogic\WorldObject.h"
#include "Graphics\Shape.h"

namespace Plutonium
{
	/* Defines a basic multitextured model that can be placed in the world. */
	struct StaticModel
		: public WorldObject
	{
		StaticModel(_In_ const StaticModel &value) = delete;
		StaticModel(_In_ StaticModel &&value) = delete;
		/* Releases the resources allocated by the model. */
		~StaticModel(void);

		_Check_return_ StaticModel& operator =(_In_ const StaticModel &other) = delete;
		_Check_return_ StaticModel& operator =(_In_ StaticModel &&other) = delete;

		/* Loads a model from a specified .obj file (requires delete!). */
		_Check_return_ static StaticModel* FromFile(_In_ const char *path);

	protected:
		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct StaticRenderer;

		StaticModel(void)
			: WorldObject()
		{}

		std::vector<Shape*> shapes;
	};
}