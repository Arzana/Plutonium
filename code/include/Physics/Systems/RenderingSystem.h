#pragma once
#include "Physics/Objects/PhysicsHandle.h"
#include "Graphics/Lighting/DeferredRenderer.h"

namespace Pu
{
	class PhysicalWorld;
	class DeferredRenderer;
	class BVH;

	/* Defines a system that is responsible for rendering visible physical objects. */
	class RenderingSystem
	{
	public:
		/* Initializes a new instance of a rendering system that renders object from the specified physical world to the specified deferred renderer. */
		RenderingSystem(_In_ const PhysicalWorld &world, _In_ DeferredRenderer &renderer);
		RenderingSystem(_In_ const RenderingSystem&) = delete;
		/* Move constructor. */
		RenderingSystem(_In_ RenderingSystem &&value) = default;

		_Check_return_ RenderingSystem& operator =(_In_ const RenderingSystem&) = delete;
		/* Move assignment. */
		_Check_return_ RenderingSystem& operator =(_In_ RenderingSystem &&other) = default;

		/* Adds a terrain chunk to the renderer. */
		void Add(_In_ PhysicsHandle handle, _In_ const TerrainChunk &chunk);
		/* Adds a model to the renderer. */
		void Add(_In_ PhysicsHandle handle, _In_ const Model &model, _In_ uint32 subpass);
		/* Adds a directional light to the renderer. */
		inline void Add(_In_ const DirectionalLight &light)
		{
			dirLights.emplace_back(&light);
		}

		/* Renders the current physical world state to the renderer. */
		void Render(_In_ const BVH &bvh, _In_ const Camera &camera, _In_ CommandBuffer &cmdBuffer);

		/* Removes the specified object from the renderer. */
		void Remove(_In_ PhysicsHandle handle);
		/* Removes the specified directional light from the rendering queue. */
		inline void Remove(_In_ const DirectionalLight &light)
		{
			dirLights.remove(&light);
		}

	private:
		using PhysicsHandlePair = std::pair<Pu::PhysicsHandle, Pu::PhysicsHandle>;

		const PhysicalWorld *world;
		DeferredRenderer *renderer;
		std::map<PhysicsHandle, PhysicsHandle> handleLut;

		vector<const DirectionalLight*> dirLights;
		vector<const TerrainChunk*> terrains;
		vector<std::pair<const Model*, uint32>> models;

		mutable vector<PhysicsHandle> cacheCast;
		mutable vector<PhysicsHandlePair> cacheHandles;
		mutable vector<std::pair<PhysicsHandle, const Asset*>> loadingAssets;

		void CheckLoadingAssets(void);
		void AddHandleToLuT(PhysicsHandle handle, size_t idx, uint32 subpass);
		void UpdateCaches(const BVH &bvh, const Camera &cam);
	};
}