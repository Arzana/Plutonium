#pragma once
#include "Graphics\PhongMaterial.h"
#include "Graphics\Native\Window.h"

namespace Plutonium
{
	struct AssetLoader;

	/* Defines a basic multitextured model that can be placed in the world. */
	struct StaticModel
	{
		/* Creates a simple static model with one material (material wil be deleted by the model). */
		StaticModel(_In_ PhongMaterial *material);
		StaticModel(_In_ const StaticModel &value) = delete;
		StaticModel(_In_ StaticModel &&value) = delete;
		/* Releases the resources allocated by the model. */
		~StaticModel(void);

		_Check_return_ StaticModel& operator =(_In_ const StaticModel &other) = delete;
		_Check_return_ StaticModel& operator =(_In_ StaticModel &&other) = delete;

		/* Gets the name assigned to the model. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

		/* Gets the underlying shapes of the model. */
		_Check_return_ inline const std::vector<PhongMaterial*>* GetShapes(void) const
		{
			return &shapes;
		}

	protected:
		const char *name;
		const char *path;

		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct StaticRenderer;
		friend struct DebugMeshRenderer;
		friend struct AssetLoader;

		std::vector<PhongMaterial*> shapes;
		WindowHandler wnd;

		StaticModel(WindowHandler wnd)
			: wnd(wnd)
		{}

		static StaticModel* FromFile(const char *path, AssetLoader *loader);

		int64 ContainsMaterial(const char *name);
	};
}