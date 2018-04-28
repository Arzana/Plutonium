#pragma once
#include "GameLogic\WorldObject.h"
#include "Graphics\PhongShape.h"
#include "Graphics\Native\Window.h"

namespace Plutonium
{
	struct AssetLoader;

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
		_Check_return_ static StaticModel* FromFile(_In_ const char *path, _In_ AssetLoader *loader);

	protected:
		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct StaticRenderer;

		std::vector<PhongShape*> shapes;
		WindowHandler wnd;

		StaticModel(WindowHandler wnd)
			: WorldObject(), wnd(wnd)
		{}

		int64 ContainsMaterial(const char *name);
	};
}