#pragma once
#include <atomic>
#include "Graphics\Mesh.h"
#include "Graphics\Native\Window.h"

namespace Plutonium
{
	class AssetLoader;
	class MaterialBP;

	/* Defines a basic multitextured model that can be placed in the world. */
	class StaticModel
	{
	public:
		/* Defines a shape within the static model. */
		struct Shape
		{
			/* The material associated with this shape. */
			MaterialBP *Material;
			/* The mesh associated with this shape. */
			Mesh *Mesh;
		};

		/* Creates a simple static model with one material (material wil be deleted by the model). */
		StaticModel(_In_ MaterialBP *material, _In_ Mesh *mesh);
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
		_Check_return_ inline const std::vector<Shape>* GetShapes(void) const
		{
			return &shapes;
		}

	protected:
		const char *name;
		const char *path;

		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend class AssetLoader;

		std::vector<Shape> shapes;
		WindowHandler wnd;

		StaticModel(WindowHandler wnd)
			: wnd(wnd)
		{}

		static StaticModel* FromFile(const char *path, AssetLoader *loader, std::atomic<float> *progression = nullptr);

		int64 ContainsMaterial(const char *name);
	};
}