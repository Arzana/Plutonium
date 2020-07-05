#include "Physics/Systems/ConstraintSystem.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Math/HeightMap.h"

#define create_collision_t(first, second)		(static_cast<Pu::uint16>(static_cast<Pu::uint16>(first) | static_cast<Pu::uint16>(second) << 8))
#define as_sphere(params)						(*reinterpret_cast<const Pu::Sphere*>(params))
#define as_height(params)						(*reinterpret_cast<const Pu::HeightMap*>(params))

static Pu::uint32 narrowPhaseChecks = 0;

Pu::ConstraintSystem::ConstraintSystem(PhysicalWorld & world)
	: world(&world)
{
	SetGenericCheckers();
}

Pu::ConstraintSystem::ConstraintSystem(ConstraintSystem && value)
	: checkers(std::move(value.checkers)), world(value.world),
	rawBroadPhase(std::move(value.rawBroadPhase)),
	cachedBroadPhase(std::move(value.cachedBroadPhase)), 
	rawNarrowPhase(std::move(value.rawNarrowPhase))
{
	SetGenericCheckers();
}

Pu::ConstraintSystem & Pu::ConstraintSystem::operator=(ConstraintSystem && other)
{
	if (this != &other)
	{
		Destroy();

		world = other.world;
		rawBroadPhase = std::move(other.rawBroadPhase);
		cachedBroadPhase = std::move(other.cachedBroadPhase);
		rawNarrowPhase = std::move(other.rawNarrowPhase);
	}

	return *this;
}

Pu::uint32 Pu::ConstraintSystem::GetNarrowPhaseChecks(void)
{
	return narrowPhaseChecks;
}

void Pu::ConstraintSystem::ResetCounter(void)
{
	narrowPhaseChecks = 0;
}

void Pu::ConstraintSystem::AddItem(PhysicsHandle handle, const AABB & bb, CollisionShapes type, const float * collider)
{
	AABB bb2 = bb * world->GetTransform(handle);

	/* This system need to check if kinematic objects collide with others, so handle them seperately. */
	if (physics_get_type(handle) == PhysicsType::Kinematic)
	{
		bb2.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);
		cachedBroadPhase.emplace(handle, bb2);
	}

	/* Add the broadphase to the BVH and emplace the collider type. */
	world->searchTree.Insert(handle, bb2);
	rawBroadPhase.emplace_back(bb);

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
		Log::Error("ConstraintSystem cannot handle collider of type %s currently!", to_string(type));
		return;
	}

	rawNarrowPhase.emplace(handle, std::make_pair(type, copy));
}

void Pu::ConstraintSystem::RemoveItem(PhysicsHandle handle)
{
	/* Remove the bounding boxes from the cache buffer if it's kinematic. */
	if (physics_get_type(handle) == PhysicsType::Kinematic)
	{
		cachedBroadPhase.erase(handle);
	}

	world->searchTree.Remove(handle);

	/* Make sure to free the narrow phase. */
	const uint16 idx = world->QueryInternalIndex(handle);
	rawBroadPhase.removeAt(idx);
	free(rawNarrowPhase[handle].second);
	rawNarrowPhase.erase(handle);
}

void Pu::ConstraintSystem::Check(void)
{
	/* Query the movement system for updates to the BVH. */
	readdCache.clear();
	world->sysMove->CheckDistance(readdCache);
	for (auto[idx, pos] : readdCache)
	{
		/* Remove the old bounding box from the BVH. */
		const PhysicsHandle hobj = world->QueryPublicHandle(create_physics_handle(PhysicsType::Kinematic, idx));
		world->searchTree.Remove(hobj);

		/* Create the new cached bounding box. */
		AABB newBB = rawBroadPhase[physics_get_lookup_id(hobj)] + pos;
		newBB.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);

		/* Insert the new bounding box. */
		cachedBroadPhase[hobj] = newBB;
		world->searchTree.Insert(hobj, newBB);
	}

	/* Check for collisions. */
	for (const auto &[hobj, bb] : cachedBroadPhase)
	{
		/* Traverse the BVH to perform broad phase for this kinematic object. */
		broadPhaseCache.clear();
		world->searchTree.Boxcast(bb, broadPhaseCache);

		/* Perform narrow phase for all the hits, ignoring self. */
		for (const PhysicsHandle hhit : broadPhaseCache)
		{
			if (hhit != hobj) TestGeneric(hhit, hobj);
		}
	}
}

void Pu::ConstraintSystem::Visualize(DebugRenderer & dbgRenderer, Vector3 camPos) const
{
	/* Display yellow for cached broadphases. */
	for (const auto[hcur, bb] : cachedBroadPhase)
	{
		dbgRenderer.AddBox(bb, Color::Yellow());
	}

	for (const auto[hcur, narrow] : rawNarrowPhase)
	{
		const Color clr = physics_get_type(hcur) == PhysicsType::Static ? Color::Green() : Color::Red();
		const Matrix transform = world->GetTransform(hcur);

		if (narrow.first == CollisionShapes::Sphere) dbgRenderer.AddSphere(as_sphere(narrow.second) * transform, clr);
		else if (narrow.first == CollisionShapes::HeightMap)
		{
			const HeightMap &collider = as_height(narrow.second);
			const Vector3 offset = transform.GetTranslation();

			if (collider.Contains(Vector2(camPos.X - offset.X, camPos.Z - offset.Z))) collider.Visualize(dbgRenderer, offset, clr);
		}
	}
}

void Pu::ConstraintSystem::TestGeneric(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	++narrowPhaseChecks;
	const CollisionShapes shape1 = rawNarrowPhase[hfirst].first;
	const CollisionShapes shape2 = rawNarrowPhase[hsecond].first;

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

void Pu::ConstraintSystem::TestSphereSphere(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	/* Query the colliders and transform them to the correct position. */
	const Sphere sphere1 = as_sphere(rawNarrowPhase[hfirst].second) * world->GetTransform(hfirst);
	const Sphere sphere2 = as_sphere(rawNarrowPhase[hsecond].second) * world->GetTransform(hsecond);

	/* Check for collision. */
	if (intersects(sphere1, sphere2))
	{
		world->sysSolv->RegisterCollision(CollisionManifold{ hfirst, hsecond, dir(sphere1.Center, sphere2.Center) });
	}
}

void Pu::ConstraintSystem::TestAABBSphere(PhysicsHandle haabb, PhysicsHandle hsphere)
{
	/* Query the colliders and transform them to the correct position. */
	const AABB aabb = rawBroadPhase[haabb] * world->GetTransform(haabb);
	const Sphere sphere = as_sphere(rawNarrowPhase[hsphere].second) * world->GetTransform(hsphere);

	/* Check for collision. */
	const Vector3 q = closest(aabb, sphere.Center);
	if (sqrdist(q, sphere.Center) < sqr(sphere.Radius))
	{
		world->sysSolv->RegisterCollision(CollisionManifold{ haabb, hsphere, dir(q, sphere.Center) });
	}
}

void Pu::ConstraintSystem::TestHeightmapSphere(PhysicsHandle hmap, PhysicsHandle hsphere)
{
	/* Query the colliders and transform them to the correct position. */
	const HeightMap &heightmap = as_height(rawNarrowPhase[hmap].second);
	const Vector3 offset = world->GetTransform(hmap).GetTranslation();
	const Sphere sphere = as_sphere(rawNarrowPhase[hsphere].second) * world->GetTransform(hsphere);

	/* Query the heightmap for the height and normal under the sphere. */
	float h;
	Vector3 n;
	if (heightmap.TryGetHeightAndNormal(Vector2(sphere.Center.X - offset.X, sphere.Center.Z - offset.Z), h, n))
	{
		/* The sphere collides with the heightmap is it's lowest point is below the sample height. */
		if (h >= sphere.Center.Y - sphere.Radius)
		{
			world->sysSolv->RegisterCollision(CollisionManifold{ hmap, hsphere, n });
		}
	}
}

void Pu::ConstraintSystem::SetGenericCheckers(void)
{
	checkers.emplace(create_collision_t(CollisionShapes::None, CollisionShapes::Sphere), &ConstraintSystem::TestAABBSphere);
	checkers.emplace(create_collision_t(CollisionShapes::Sphere, CollisionShapes::Sphere), &ConstraintSystem::TestSphereSphere);
	checkers.emplace(create_collision_t(CollisionShapes::HeightMap, CollisionShapes::Sphere), &ConstraintSystem::TestHeightmapSphere);
}

void Pu::ConstraintSystem::Destroy(void)
{
	for (auto [hcur, narrow] : rawNarrowPhase) free(narrow.second);
}