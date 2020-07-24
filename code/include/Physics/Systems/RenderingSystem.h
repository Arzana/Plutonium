#pragma once
#include "Physics/Objects/BVH.h"
#include "Physics/Objects/PhysicsHandle.h"
#include "Graphics/Lighting/DeferredRenderer.h"

namespace Pu
{
	class PhysicalWorld;

	/* Defines a system that is responsible for rendering visible physical objects. */
	class RenderingSystem
	{
	public:
		/* Initializes a new instance of a rendering system that renders object from the specified physical world to the specified deferred renderer. */
		RenderingSystem(_In_ const PhysicalWorld &world, _In_ DeferredRenderer &renderer);
		RenderingSystem(_In_ const RenderingSystem&) = delete;
		/* Move constructor. */
		RenderingSystem(_In_ RenderingSystem &&value);
		/* Releases the resources allocated by the rendering system. */
		~RenderingSystem(void)
		{
			Destroy();
		}

		_Check_return_ RenderingSystem& operator =(_In_ const RenderingSystem&) = delete;
		/* Move assignment. */
		_Check_return_ RenderingSystem& operator =(_In_ RenderingSystem &&other);

		/* Adds a terrain chunk to the renderer. */
		void Add(_In_ PhysicsHandle handle, _In_ const TerrainChunk &chunk);
		/* Adds a model to the renderer. */
		void Add(_In_ PhysicsHandle handle, _In_ const Model &model, _In_ uint32 subpass);
		/* Adds a point light to the renderer. */
		_Check_return_ PhysicsHandle Add(_In_ const PointLight &light);
		/* Adds a directional light to the renderer. */
		_Check_return_ PhysicsHandle Add(_In_ const DirectionalLight &light);

		/* Renders the current physical world state to the renderer. */
		void Render(_In_ const BVH &bvh, _In_ const Camera &camera, _In_ CommandBuffer &cmdBuffer);
		/* Removes the specified object from the renderer. */
		void Remove(_In_ PhysicsHandle handle);

		/* Gets the visual BVH. */
		_Check_return_ inline const BVH& GetVisualBVH(void) const
		{
			return visualTree;
		}

	private:
		const PhysicalWorld *world;
		DeferredRenderer *renderer;
		std::map<PhysicsHandle, PhysicsHandle> handleLut;

		BVH visualTree;
		vector<PointLightPool*> pntLightPools;

		vector<const DirectionalLight*> dirLights;
		vector<const TerrainChunk*> terrains;
		vector<std::pair<const Model*, uint32>> models;
		vector<PointLight> pntLights;

		mutable vector<PhysicsHandle> cacheCast;
		mutable vector<PhysicsHandlePair> cacheHandles;
		mutable vector<std::pair<PhysicsHandle, const Asset*>> loadingAssets;

		void CheckLoadingAssets(void);
		PhysicsHandle AllocLightHandle(uint32 subpass);
		void AddHandleToLuT(PhysicsHandle handle, size_t idx, uint32 subpass);
		void UpdateCaches(const BVH &bvh, const Camera &cam);
		void Destroy(void);
	};
}