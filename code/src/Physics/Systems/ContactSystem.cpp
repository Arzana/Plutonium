#include "Physics/Systems/ContactSystem.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Physics/Systems/MovementSystem.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/HeightMap.h"

#define create_collision_t(first, second)		(static_cast<Pu::uint16>(static_cast<Pu::uint16>(first) | static_cast<Pu::uint16>(second) << 8))
#define as_sphere(params)						(*reinterpret_cast<const Pu::Sphere*>(params))
#define as_height(params)						(*reinterpret_cast<const Pu::HeightMap*>(params))

static Pu::uint32 bvhUpdateCalls = 0;
static Pu::uint32 narrowPhaseChecks = 0;
static Pu::uint32 collisionCount = 0;

Pu::ContactSystem::ContactSystem(PhysicalWorld & world)
	: world(&world)
{
	SetGenericCheckers();
}

Pu::ContactSystem::ContactSystem(ContactSystem && value)
	: checkers(std::move(value.checkers)), world(value.world),
	rawBroadPhase(std::move(value.rawBroadPhase)),
	cachedBroadPhase(std::move(value.cachedBroadPhase)),
	rawNarrowPhase(std::move(value.rawNarrowPhase)),
	hfirsts(std::move(value.hfirsts)), hseconds(std::move(value.hseconds)),
	nx(std::move(value.nx)), ny(std::move(value.ny)), nz(std::move(value.nz)),
	px(std::move(value.px)), py(std::move(value.py)), pz(std::move(value.pz))
{
	SetGenericCheckers();
}

Pu::ContactSystem & Pu::ContactSystem::operator=(ContactSystem && other)
{
	if (this != &other)
	{
		Destroy();

		hfirsts = std::move(other.hfirsts);
		hseconds = std::move(other.hseconds);
		px = std::move(other.px);
		py = std::move(other.py);
		pz = std::move(other.pz);
		nx = std::move(other.nx);
		ny = std::move(other.ny);
		nz = std::move(other.nz);

		world = other.world;
		rawBroadPhase = std::move(other.rawBroadPhase);
		cachedBroadPhase = std::move(other.cachedBroadPhase);
		rawNarrowPhase = std::move(other.rawNarrowPhase);
	}

	return *this;
}

Pu::uint32 Pu::ContactSystem::GetBVHUpdateCalls(void)
{
	return bvhUpdateCalls;
}

Pu::uint32 Pu::ContactSystem::GetNarrowPhaseChecks(void)
{
	return narrowPhaseChecks;
}

Pu::uint32 Pu::ContactSystem::GetCollisionsCount(void)
{
	return collisionCount;
}

void Pu::ContactSystem::ResetCounters(void)
{
	bvhUpdateCalls = 0;
	narrowPhaseChecks = 0;
	collisionCount = 0;
}

void Pu::ContactSystem::AddItem(PhysicsHandle handle, const AABB & bb, CollisionShapes type, const float * collider)
{
	AABB bb2 = bb * world->GetTransform(handle);
	++bvhUpdateCalls;

	/* This system need to check if kinematic objects collide with others, so handle them seperately. */
	if (physics_get_type(handle) == PhysicsType::Kinematic)
	{
		bb2.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);
		rawBroadPhase.emplace_back(bb);
	}

	/* Add the broadphase to the BVH and emplace the collider type. */
	world->searchTree.Insert(handle, bb2);
	cachedBroadPhase.emplace(handle, bb2);

	/* We need to make a copy of the collider incase the user defined it in stack memory. */
	float *copy = nullptr;
	switch (type)
	{
	case CollisionShapes::None:
		if (physics_get_type(handle) == PhysicsType::Kinematic)
		{
			Log::Error("Kinematic objects cannot have an AABB collider!");
			return;
		}
		break;
	case CollisionShapes::Sphere:
		copy = reinterpret_cast<float*>(malloc(sizeof(Sphere)));
		memcpy(copy, collider, sizeof(Sphere));
		break;
	case CollisionShapes::HeightMap:
		copy = reinterpret_cast<float*>(malloc(sizeof(HeightMap)));
		new(copy) HeightMap(*reinterpret_cast<const HeightMap*>(collider));
		break;
	default:
		Log::Error("ContactSystem cannot handle collider of type %s currently!", to_string(type));
		return;
	}

	rawNarrowPhase.emplace(handle, std::make_pair(type, copy));
}

void Pu::ContactSystem::RemoveItem(PhysicsHandle handle)
{
	cachedBroadPhase.erase(handle);
	world->searchTree.Remove(handle);
	++bvhUpdateCalls;

	/* Make sure to free the narrow phase. */
	const uint16 idx = world->QueryInternalIndex(handle);
	if (physics_get_type(handle) == PhysicsType::Kinematic) rawBroadPhase.removeAt(idx);
	free(rawNarrowPhase.at(handle).second);
	rawNarrowPhase.erase(handle);
}

void Pu::ContactSystem::Check(void)
{
	/* Remove the previous collisions from the buffer. */
	hfirsts.clear();
	hseconds.clear();
	nx.clear();
	ny.clear();
	nz.clear();

	if constexpr (PhysicsProfileSystems) Profiler::Begin("BVH Update", Color::Abbey());

	/* Query the movement system for updates to the BVH. */
	readdCache.clear();
	world->sysMove->CheckDistance(readdCache);
	for (auto[idx, pos] : readdCache)
	{
		/* Remove the old bounding box from the BVH. */
		const PhysicsHandle hobj = world->QueryPublicHandle(create_physics_handle(PhysicsType::Kinematic, idx));
		world->searchTree.Remove(hobj);

		/* Create the new cached bounding box. */
		AABB newBB = rawBroadPhase[idx] + pos;
		newBB.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);

		/* Insert the new bounding box. */
		cachedBroadPhase.at(hobj) = newBB;
		world->searchTree.Insert(hobj, newBB);
		++bvhUpdateCalls;
	}

	if constexpr (PhysicsProfileSystems) Profiler::End();

	/* Check for collisions. */
	for (const auto &[hobj, bb] : cachedBroadPhase)
	{
		/* We don't have to check sleeping or static objects. */
		if (physics_get_type(hobj) == PhysicsType::Static) continue;
		if (world->sysMove->IsSleeping(world->QueryInternalIndex(hobj))) continue;

		/* Traverse the BVH to perform broad phase for this kinematic object. */
		if constexpr (PhysicsProfileSystems) Profiler::Begin("Broadphase", Color::Crimson());
		broadPhaseCache.clear();
		world->searchTree.Boxcast(bb, broadPhaseCache);

		if constexpr (PhysicsProfileSystems)
		{
			Profiler::End();
			Profiler::Begin("Narrowphase", Color::Scarlet());
		}

		/* Perform narrow phase for all the hits, ignoring self. */
		for (const PhysicsHandle hhit : broadPhaseCache)
		{
			if (hhit != hobj) TestGeneric(hhit, hobj);
		}

		if constexpr (PhysicsProfileSystems) Profiler::End();
	}
}

#ifdef _DEBUG
void Pu::ContactSystem::Visualize(DebugRenderer & dbgRenderer, Vector3 camPos) const
{
	/* Display yellow for cached broadphases. */
	for (const auto[hcur, bb] : cachedBroadPhase)
	{
		if (physics_get_type(hcur) == PhysicsType::Kinematic) dbgRenderer.AddBox(bb, Color::Yellow());
	}

	for (const auto[hcur, narrow] : rawNarrowPhase)
	{
		/*
		Static objects are displayed as green (to indicate that they are fast to compute).
		Kinematic ojects are displayed as red (they have all physics systems on them).
		*/
		const Color clr = physics_get_type(hcur) == PhysicsType::Static ? Color::Green() : Color::Red();
		const Matrix transform = world->GetTransform(hcur);

		if (narrow.first == CollisionShapes::None) dbgRenderer.AddBox(cachedBroadPhase.at(hcur), clr);
		else if (narrow.first == CollisionShapes::Sphere) dbgRenderer.AddSphere(as_sphere(narrow.second) * transform, clr);
		else if (narrow.first == CollisionShapes::HeightMap)
		{
			const HeightMap &collider = as_height(narrow.second);
			const Vector3 offset = transform.GetTranslation();

			/* The heightmap is incredibly expensive to debug render, so only do it for one. */
			if (collider.Contains(Vector2(camPos.X - offset.X, camPos.Z - offset.Z))) collider.Visualize(dbgRenderer, offset, clr);
			else dbgRenderer.AddBox(cachedBroadPhase.at(hcur), clr);
		}
	}
}
#endif

void Pu::ContactSystem::TestGeneric(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	++narrowPhaseChecks;
	const CollisionShapes shape1 = rawNarrowPhase.at(hfirst).first;
	const CollisionShapes shape2 = rawNarrowPhase.at(hsecond).first;

	/*
	Construct a key from the collision type, then check if it's in the list.
	If not, try again, but with reverse order.
	Otherwise it's an invalid collision.
	*/
	decltype(checkers)::iterator it = checkers.find(create_collision_t(shape1, shape2));
	if (it != checkers.end())
	{
		((*this).*it->second)(hfirst, hsecond);
		return;
	}

	it = checkers.find(create_collision_t(shape2, shape1));
	if (it != checkers.end())
	{
		((*this).*it->second)(hsecond, hfirst);
		return;
	}

	Log::Warning("Unable to check for collision between %s and %s!", to_string(shape1), to_string(shape2));
}

void Pu::ContactSystem::TestSphereSphere(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	/* Query the colliders and transform them to the correct position. */
	const Sphere sphere1 = as_sphere(rawNarrowPhase.at(hfirst).second) * world->GetTransform(hfirst);
	const Sphere sphere2 = as_sphere(rawNarrowPhase.at(hsecond).second) * world->GetTransform(hsecond);

	/* Check for collision. */
	if (intersects(sphere1, sphere2))
	{
		const Vector3 n = dir(sphere1.Center, sphere2.Center);
		const Vector3 p = sphere1.Center + sphere1.Radius * n;
		AddManifold(hfirst, hsecond, p, n);
	}
}

void Pu::ContactSystem::TestAABBSphere(PhysicsHandle haabb, PhysicsHandle hsphere)
{
	/* Query the sphere collider and transform it to the correct position. */
	const Sphere sphere = as_sphere(rawNarrowPhase.at(hsphere).second) * world->GetTransform(hsphere);

	/* Check for collision with the cached bounding box. */
	const Vector3 q = closest(cachedBroadPhase.at(haabb), sphere.Center);
	if (sqrdist(q, sphere.Center) < sqr(sphere.Radius))
	{
		const Vector3 n = dir(q, sphere.Center);
		const Vector3 p = sphere.Center + sphere.Radius * -n;
		AddManifold(haabb, hsphere, p, n);
	}
}

void Pu::ContactSystem::TestHeightmapSphere(PhysicsHandle hmap, PhysicsHandle hsphere)
{
	/* Query the colliders and transform them to the correct position. */
	const HeightMap &heightmap = as_height(rawNarrowPhase.at(hmap).second);
	const Vector3 offset = world->GetTransform(hmap).GetTranslation();
	const Sphere sphere = as_sphere(rawNarrowPhase.at(hsphere).second) * world->GetTransform(hsphere);

	float h;
	Vector3 n;

	/* Query the heightmap for the height and normal under the sphere. */
	const Vector2 query = Vector2(sphere.Center.X - offset.X, sphere.Center.Z - offset.Z);
	if (heightmap.TryGetHeightAndNormal(query, h, n))
	{
		/* The sphere collides with the heightmap is it's lowest point is below the sample height. */
		if (h >= sphere.Center.Y - sphere.Radius) AddManifold(hmap, hsphere, Vector3(query.X, h, query.Y), n);
	}
}

void Pu::ContactSystem::AddManifold(PhysicsHandle hfirst, PhysicsHandle hsecond, Vector3 pos, Vector3 normal)
{
	/* Ignore any duplicate collisions. */
	for (size_t i = 0; i < hfirsts.size(); i++)
	{
		if (hfirsts[i] == hsecond && hseconds[i] == hfirst) return;
	}

	/* Ignore the collision if we have a seperating velocity (i.e. moving away from each other). */
	if (physics_get_type(hfirst) != PhysicsType::Static)
	{
		const Vector3 v1 = world->sysMove->GetVelocity(world->QueryInternalIndex(hfirst));
		const Vector3 v2 = world->sysMove->GetVelocity(world->QueryInternalIndex(hsecond));

		if (dot(v2 - v1, normal) > 0.0f) return;
	}

	hfirsts.emplace_back(hfirst);
	hseconds.emplace_back(hsecond);
	px.push(pos.X);
	py.push(pos.Y);
	pz.push(pos.Z);
	nx.push(normal.X);
	ny.push(normal.Y);
	nz.push(normal.Z);

	++collisionCount;
}

void Pu::ContactSystem::SetGenericCheckers(void)
{
	checkers.emplace(create_collision_t(CollisionShapes::None, CollisionShapes::Sphere), &ContactSystem::TestAABBSphere);
	checkers.emplace(create_collision_t(CollisionShapes::Sphere, CollisionShapes::Sphere), &ContactSystem::TestSphereSphere);
	checkers.emplace(create_collision_t(CollisionShapes::HeightMap, CollisionShapes::Sphere), &ContactSystem::TestHeightmapSphere);
}

void Pu::ContactSystem::Destroy(void)
{
	for (auto[hcur, narrow] : rawNarrowPhase) free(narrow.second);
}