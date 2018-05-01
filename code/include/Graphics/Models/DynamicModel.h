#pragma once
#include "Graphics\Mesh.h"
#include "Graphics\Texture.h"
#include "AnimationInfo.h"
#include "GameLogic\WorldObject.h"

namespace Plutonium
{
	/* Defines a function template for a animation initializer function. */
	using Initializer = void(*)(_In_ const char *Name, _Out_ PlayBackFlags &flags, _Out_ float &fps);

	/* Defines a basic animated model that can be placed in the world. */
	struct DynamicModel
	{
	public:
		DynamicModel(_In_ const DynamicModel &value) = delete;
		DynamicModel(_In_ DynamicModel &&value) = delete;
		/* Releases the resources allocated by the model. */
		~DynamicModel(void);

		_Check_return_ DynamicModel& operator =(_In_ const DynamicModel &other) = delete;
		_Check_return_ DynamicModel& operator =(_In_ DynamicModel &&other) = delete;

		/* Gets the name assigned to the model. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

	protected:
		std::vector<AnimationInfo*> animations;
		Texture *skin;
		const char *name;
		const char *path;

		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct DynamicRenderer;
		friend struct WireframeRenderer;
		friend struct DynamicObject;
		friend struct AssetLoader;

		AssetLoader *loader;

		DynamicModel(AssetLoader *loader);

		static DynamicModel* FromFile(const char *path, AssetLoader *loader, const char *texture = nullptr);

		void SplitFrames(std::vector<Mesh*> meshes);
	};
}