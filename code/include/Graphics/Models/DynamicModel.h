#pragma once
#include <atomic>
#include "Graphics\Mesh.h"
#include "AnimationInfo.h"
#include "GameLogic\WorldObject.h"

namespace Plutonium
{
	/* Defines a function template for a animation initializer function. */
	using Initializer = void(*)(_In_ const char *Name, _Out_ PlayBackFlags &flags, _Out_ float &fps);

	struct MaterialBP;

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

		/* Gets the material of this model. */
		_Check_return_ inline const MaterialBP* GetMaterial(void) const
		{
			return material;
		}

	protected:
		std::vector<AnimationInfo*> animations;
		MaterialBP *material;
		const char *name;
		const char *path;

		/* Finalizes the underlying meshes, sending them to the GPU. */
		void Finalize(void);

	private:
		friend struct DynamicRenderer;
		friend struct DynamicObject;
		friend struct DebugMeshRenderer;
		friend struct AssetLoader;

		AssetLoader *loader;

		DynamicModel(AssetLoader *loader);

		static DynamicModel* FromFile(const char *path, AssetLoader *loader, const char *texture = nullptr, std::atomic<float> *progression = nullptr);

		void SplitFrames(std::vector<Mesh*> meshes);
	};
}