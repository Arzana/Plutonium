#include "Physics/Systems/RenderingSystem.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Core/Diagnostics/Profiler.h"

constexpr Pu::uint32 PointLightPoolSize = 256;

constexpr inline Pu::uint32 physics_get_subpass(Pu::PhysicsHandle handle)
{
	return handle >> 16 & 0xF;
}

constexpr inline Pu::PhysicsHandle physics_clear_subpass(Pu::PhysicsHandle handle)
{
	return handle & ~Pu::PhysicsHandleImplBits;
}

constexpr inline void physics_set_subpass(Pu::PhysicsHandle &handle, Pu::uint32 subpass)
{
#ifdef _DEBUG
	if (subpass > 0xF) Pu::Log::Fatal("Attempting to set subpass out of handle range!");
#endif

	handle |= subpass << 16;
}

constexpr inline bool physics_handle_sort_pair(const Pu::PhysicsHandlePair &first, const Pu::PhysicsHandlePair &second)
{
	return physics_get_subpass(first.second) < physics_get_subpass(second.second);
}

Pu::RenderingSystem::RenderingSystem(const PhysicalWorld & world, DeferredRenderer & renderer)
	: world(&world), renderer(&renderer)
{}

Pu::RenderingSystem::RenderingSystem(RenderingSystem && value)
	: world(value.world), renderer(value.renderer), handleLut(std::move(value.handleLut)),
	visualTree(std::move(value.visualTree)), pntLightPools(std::move(value.pntLightPools)),
	dirLights(std::move(value.dirLights)), terrains(std::move(value.terrains)),
	models(std::move(value.models)), pntLights(std::move(pntLights)),
	cacheCast(std::move(cacheCast)), cacheHandles(std::move(value.cacheHandles)),
	loadingAssets(std::move(value.loadingAssets))
{}

Pu::RenderingSystem & Pu::RenderingSystem::operator=(RenderingSystem && other)
{
	if (this != &other)
	{
		Destroy();

		world = other.world;
		renderer = other.renderer;
		handleLut = std::move(other.handleLut);
		visualTree = std::move(other.visualTree);
		pntLightPools = std::move(other.pntLightPools);
		dirLights = std::move(other.dirLights);
		terrains = std::move(other.terrains);
		models = std::move(other.models);
		pntLights = std::move(other.pntLights);
		cacheCast = std::move(other.cacheCast);
		cacheHandles = std::move(other.cacheHandles);
		loadingAssets = std::move(other.loadingAssets);
	}

	return *this;
}

void Pu::RenderingSystem::Add(PhysicsHandle handle, const TerrainChunk & chunk)
{
	const size_t idx = terrains.size();

	/* Terrain chunks are always added to the list as they are unique. */
	if (chunk.IsLoaded()) terrains.emplace_back(&chunk);
	else
	{
		physics_set_subpass(handle, DeferredRenderer::SubpassTerrain);
		loadingAssets.emplace_back(std::make_pair(handle, static_cast<const Asset*>(&chunk)));
		return;
	}

	AddHandleToLuT(handle, idx, DeferredRenderer::SubpassTerrain);
}

void Pu::RenderingSystem::Add(PhysicsHandle handle, const Model & model, uint32 subpass)
{
	/* Validate the user defined subpass. */
#ifdef _DEBUG
	if (subpass > DeferredRenderer::SubpassPostProcessing) Log::Fatal("Unknown subpass %u passed to RenderingSystem::Add!", subpass);
#endif

	size_t idx;

	/* Models are not unique, so check if this model was already added. */
	decltype(models)::const_iterator it = models.iteratorOf([&model](const std::pair<const Model*, uint32> &cur) { return cur.first == &model; });
	if (it != models.end()) idx = std::distance(models.cbegin(), it);
	else
	{
		idx = models.size();

		/* The model is done loading and can be added right away. */
		if (model.IsLoaded()) models.emplace_back(std::make_pair(&model, 1));
		else
		{
			/* The model isn't done loading yet, add it to a temporary list and don't add the handle to the lookup yet. */
			physics_set_subpass(handle, subpass);
			loadingAssets.emplace_back(std::make_pair(handle, static_cast<const Asset*>(&model)));
			return;
		}
	}

	AddHandleToLuT(handle, idx, subpass);
}

Pu::PhysicsHandle Pu::RenderingSystem::Add(const PointLight & light)
{
	/* The light volume is a sphere and thusly the scale is uniform across all axis. */
	const float ir = -light.Volume.GetRight().X;
	const float d = light.Volume.GetRight().X * 2.0f;
	const AABB bb{ ir, ir, ir, d, d, d };

	const size_t idx = pntLights.size();
	const PhysicsHandle hpublic = AllocLightHandle(DeferredRenderer::SubpassPointLight);

	/* Point lights don't have to be loaded, so we can add them right away. */
	pntLights.emplace_back(light);
	AddHandleToLuT(hpublic, idx, DeferredRenderer::SubpassPointLight);
	visualTree.Insert(hpublic, bb + light.Volume.GetTranslation());

	/* Add a new pool if needed. */
	if (!(idx % PointLightPoolSize)) pntLightPools.emplace_back(new PointLightPool(renderer->GetDevice(), PointLightPoolSize));
	return hpublic;
}

Pu::PhysicsHandle Pu::RenderingSystem::Add(const DirectionalLight & light)
{
	const size_t idx = dirLights.size();
	const PhysicsHandle hpublic = AllocLightHandle(DeferredRenderer::SubpassDirectionalLight);

	/* Directional light aren't in the tree so we can just add and return. */
	dirLights.emplace_back(&light);
	AddHandleToLuT(hpublic, idx, DeferredRenderer::SubpassDirectionalLight);
	return hpublic;
}

void Pu::RenderingSystem::Render(const BVH & bvh, const Camera & camera, CommandBuffer & cmdBuffer)
{
	/* Handle all the visual-only objects. */
	if constexpr (ProfileWorldSystems) Profiler::Begin("Culling", Color::Abbey());
	UpdateCaches(visualTree, camera);

	if constexpr (ProfileWorldSystems)
	{
		Profiler::End();
		Profiler::Begin("Batching", Color::Gray());
	}

	size_t pntLightCnt = 0, pntLightIdx = 0;
	for (const PhysicsHandlePair &handles : cacheHandles)
	{
		const uint32 subpass = physics_get_subpass(handles.second);
		const uint16 i = physics_get_lookup_id(handles.second);

		if (subpass == DeferredRenderer::SubpassPointLight)
		{
			/* Add the light to the correct pool. */
			pntLightPools[pntLightIdx]->AddLight(pntLights[i]);
			if (++pntLightCnt >= PointLightPoolSize)
			{
				pntLightCnt = 0;
				++pntLightIdx;
			}
		}
		else Log::Warning("RenderingSystem is unable to render object 0x%X in subpass %u!", handles.first, subpass);
	}

	/* Stage the point light pool. */
	for (PointLightPool *pool : pntLightPools) pool->Update(cmdBuffer);

	if constexpr (ProfileWorldSystems)
	{
		Profiler::End();
		Profiler::Begin("Culling", Color::Abbey());
	}

	CheckLoadingAssets();
	UpdateCaches(bvh, camera);

	if constexpr (ProfileWorldSystems)
	{
		Profiler::End();
		Profiler::Begin("Rendering", Color::Red());
	}

	/* Begin the render sequence. */
	renderer->InitializeResources(cmdBuffer, camera);
	for (const PhysicsHandlePair &handles : cacheHandles)
	{
		const uint32 subpass = physics_get_subpass(handles.second);
		const uint16 i = physics_get_lookup_id(handles.second);

		if (subpass == DeferredRenderer::SubpassTerrain)
		{
			/* Render all the terrain chunks. */
			renderer->Begin(subpass);
			if (i < terrains.size()) renderer->Render(*terrains[i]);
		}
		else if (subpass == DeferredRenderer::SubpassBasicStaticGeometry || subpass == DeferredRenderer::SubpassAdvancedStaticGeometry)
		{
			/* The basic and advanced static geometry uses the same functions, so we can group them. */
			renderer->Begin(subpass);
			const Matrix transform = world->GetTransform(handles.first);
			renderer->Render(*models[i].first, transform);
		}
		else if (subpass == DeferredRenderer::SubpassBasicMorphGeometry)
		{
			/* TODO: Add the proper keyframe from a animation handler. */
			renderer->Begin(subpass);
			const Matrix transform = world->GetTransform(handles.first);
			renderer->Render(*models[i].first, transform, 0, 1, 0.0f);
		}
		else Log::Warning("RenderingSystem is unable to render object 0x%X in subpass %u!", handles.first, subpass);
	}

	/* Render all the directional lights. */
	for (const DirectionalLight *light : dirLights)
	{
		renderer->Begin(DeferredRenderer::SubpassDirectionalLight);
		renderer->Render(*light);
	}

	/* Render all the point lights. */
	for (PointLightPool *pool : pntLightPools)
	{
		renderer->Begin(DeferredRenderer::SubpassPointLight);
		renderer->Render(*pool);
		pool->Clear();
	}

	/* Finalize the rendering. */
	renderer->End();
	if constexpr (ProfileWorldSystems) Profiler::End();
}

void Pu::RenderingSystem::Remove(PhysicsHandle handle)
{
	const PhysicsHandle hinternal = handleLut.at(handle);
	const uint32 subpass = physics_get_subpass(hinternal);
	const uint16 idx = physics_get_lookup_id(hinternal);

	if (subpass == DeferredRenderer::SubpassTerrain) terrains.removeAt(idx);
	else if (subpass == DeferredRenderer::SubpassDirectionalLight) dirLights.removeAt(idx);
	else if (subpass == DeferredRenderer::SubpassPointLight)
	{
		visualTree.Remove(handle);
		pntLights.removeAt(idx);
	}
	else
	{
		/* Dereference the model and remove it from the list if no more references exist. */
		if (--models[idx].second < 1) models.removeAt(idx);
	}

	/* Update the private handles to match the item removal. */
	for (auto &[hpublic, hprivate] : handleLut)
	{
		hprivate -= physics_get_subpass(hprivate) == subpass && physics_get_lookup_id(hprivate) > idx;
	}

	handleLut.erase(handle);
}

void Pu::RenderingSystem::CheckLoadingAssets(void)
{
	for (size_t i = 0; i < loadingAssets.size();)
	{
		/* Check whether the current asset is done loading. */
		const auto[htmp, asset] = loadingAssets[i];
		if (asset->IsLoaded())
		{
			const PhysicsHandle hpublic = physics_clear_subpass(htmp);
			const uint32 subpass = physics_get_subpass(htmp);
			size_t idx;

			if (subpass == DeferredRenderer::SubpassTerrain)
			{
				idx = terrains.size();
				terrains.emplace_back(static_cast<const TerrainChunk*>(asset));
			}
			else
			{
				idx = models.size();
				models.emplace_back(std::make_pair(static_cast<const Model*>(asset), 1));
			}

			/* Add the handle now so the render can take care of it. */
			AddHandleToLuT(hpublic, idx, subpass);
			loadingAssets.removeAt(i);
		}
		else ++i;
	}
}

Pu::PhysicsHandle Pu::RenderingSystem::AllocLightHandle(uint32 subpass)
{
	/* Get the index + 1 of the last light source of the specified subpass. */
	size_t i = 0;
	for (const auto[hpublic, hinternal] : handleLut)
	{
		i += physics_get_type(hpublic) == PhysicsType::LightSource && physics_get_subpass(hinternal) == subpass;
	}

	/* Return a public handle. */
	return create_physics_handle(PhysicsType::LightSource, i);
}

void Pu::RenderingSystem::AddHandleToLuT(PhysicsHandle handle, size_t idx, uint32 subpass)
{
	PhysicsHandle hinternal = create_physics_handle(physics_get_type(handle), idx);
	physics_set_subpass(hinternal, subpass);
	handleLut.emplace(handle, hinternal);
}

void Pu::RenderingSystem::UpdateCaches(const BVH & bvh, const Camera & cam)
{
	/* Traverse the BVH to get all the objects visible by the camera. */
	cacheCast.clear();
	bvh.Frustumcast(cam.GetClip(), cacheCast);

	/* Convert the public handles the internal handles. */
	cacheHandles.clear();
	cacheHandles.reserve(cacheCast.size());
	for (PhysicsHandle cur : cacheCast)
	{
		decltype(handleLut)::const_iterator it = handleLut.find(cur);
		if (it != handleLut.end()) cacheHandles.emplace_back(std::make_pair(cur, it->second));
	}

	/* Sort the items basic on their rendering order. */
	cacheHandles.sort(physics_handle_sort_pair);
}

void Pu::RenderingSystem::Destroy(void)
{
	for (PointLightPool *pool : pntLightPools) delete pool;
}