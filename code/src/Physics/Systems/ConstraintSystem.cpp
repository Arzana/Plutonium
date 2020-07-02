#include "Physics/Systems/ConstraintSystem.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Math/HeightMap.h"

#define create_collision_t(first, second)		static_cast<Pu::uint16>(static_cast<Pu::uint16>(first) | static_cast<Pu::uint16>(second) << 8)

static Pu::uint32 narrowPhaseChecks = 0;

Pu::ConstraintSystem::ConstraintSystem(const MovementSystem & movement, SolverSystem & solver, BVH & bvh)
	: movement(&movement), solver(&solver), searchTree(&bvh)
{
	SetGenericCheckers();
}

Pu::ConstraintSystem::ConstraintSystem(ConstraintSystem && value)
	: checkers(std::move(value.checkers)), movement(value.movement), solver(value.solver),
	searchTree(value.searchTree), rawBroadPhase(std::move(value.rawBroadPhase)),
	cachedBroadPhase(std::move(value.cachedBroadPhase)), shapes(std::move(value.shapes)),
	narrowPhases(std::move(narrowPhases))
{
	SetGenericCheckers();
}

Pu::ConstraintSystem & Pu::ConstraintSystem::operator=(ConstraintSystem && other)
{
	if (this != &other)
	{
		Destroy();

		movement = other.movement;
		solver = other.solver;
		searchTree = other.searchTree;
		rawBroadPhase = std::move(other.rawBroadPhase);
		cachedBroadPhase = std::move(other.cachedBroadPhase);
		shapes = std::move(other.shapes);
		narrowPhases = std::move(other.narrowPhases);
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

size_t Pu::ConstraintSystem::AddItem(PhysicsHandle handle, const AABB & bb, CollisionShapes type, const float * collider)
{
	/* This system need to check if kinematic objects collide with others, so handle them seperately. */
	if (physics_get_type(handle) == PhysicsType::Kinematic)
	{
		cachedBroadPhase.emplace(handle, bb);
	}

	/* Add the broadphase to the BVH and emplace the collider type. */
	searchTree->Insert(handle, bb);
	rawBroadPhase.emplace_back(bb);
	shapes.emplace_back(type);

	/* We need to make a copy of the collider incase the user defined it in stack memory. */
	float *copy = nullptr;
	switch (type)
	{
	case CollisionShapes::None:
		if (physics_get_type(handle) == PhysicsType::Kinematic)
		{
			Log::Error("Kinematic objects cannot have an AABB collider!");
			return maxv<size_t>();
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
		return maxv<size_t>();
	}

	/* Return the index of the object. */
	narrowPhases.emplace_back(copy);
	return shapes.size() - 1;
}

void Pu::ConstraintSystem::RemoveItem(PhysicsHandle handle)
{
	/* Remove the bounding boxes from the cache buffer if it's kinematic. */
	if (physics_get_type(handle) == PhysicsType::Kinematic)
	{
		cachedBroadPhase.erase(handle);
	}

	searchTree->Remove(handle);

	/* Make sure to free the narrow phase. */
	const uint16 idx = physics_get_lookup_id(handle);
	rawBroadPhase.removeAt(idx);
	shapes.removeAt(idx);
	free(narrowPhases[idx]);
	narrowPhases.removeAt(idx);
}

void Pu::ConstraintSystem::Check(void)
{
	/* Query the movement system for updates to the BVH. */
	readdCache.clear();
	movement->CheckDistance(readdCache);
	for (auto [idx, pos] : readdCache)
	{
		/* Remove the old bounding box from the BVH. */
		const PhysicsHandle hobj = create_physics_handle(PhysicsType::Kinematic, idx);
		searchTree->Remove(hobj);

		/* Insert the new bounding box. */
		const AABB newBB = rawBroadPhase[hobj] + pos;
		cachedBroadPhase.emplace(hobj, newBB);
		searchTree->Insert(hobj, newBB);
	}

	/* Check for collisions. */
	for (const auto &[hobj, bb] : cachedBroadPhase)
	{
		/* Traverse the BVH to perform broad phase for this kinematic object. */
		broadPhaseCache.clear();
		searchTree->Boxcast(bb, broadPhaseCache);

		/* Perform narrow phase for all the hits, ignoring self. */
		for (const PhysicsHandle hhit : broadPhaseCache)
		{
			if (hhit != hobj) TestGeneric(hhit, hobj);
		}
	}
}

void Pu::ConstraintSystem::TestGeneric(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	++narrowPhaseChecks;
	const CollisionShapes shape1 = shapes[physics_get_lookup_id(hfirst)];
	const CollisionShapes shape2 = shapes[physics_get_lookup_id(hsecond)];

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
	/* Get the indices of the objects. */
	const uint16 i = physics_get_lookup_id(hfirst);
	const uint16 j = physics_get_lookup_id(hsecond);

	/* Query the colliders and transform them to the correct position. */
	const Sphere sphere1 = *reinterpret_cast<Sphere*>(narrowPhases[i]) * movement->CreateTransform(i);
	const Sphere sphere2 = *reinterpret_cast<Sphere*>(narrowPhases[j]) * movement->CreateTransform(j);

	/* Check for collision. */
	if (intersects(sphere1, sphere2))
	{
		solver->RegisterCollision(CollisionManifold{ hfirst, hsecond, dir(sphere1.Center, sphere2.Center) });
	}
}

void Pu::ConstraintSystem::TestAABBSphere(PhysicsHandle haabb, PhysicsHandle hsphere)
{
	/* Get the indices of the objects. */
	const uint16 i = physics_get_lookup_id(haabb);
	const uint16 j = physics_get_lookup_id(hsphere);

	/* Query the colliders and transform them to the correct position. */
	const AABB aabb = rawBroadPhase[i] * movement->CreateTransform(i);
	const Sphere sphere = *reinterpret_cast<Sphere*>(narrowPhases[j]) * movement->CreateTransform(j);

	/* Check for collision. */
	const Vector3 q = closest(aabb, sphere.Center);
	if (sqrdist(q, sphere.Center) < sqr(sphere.Radius))
	{
		solver->RegisterCollision(CollisionManifold{ haabb, hsphere, dir(q, sphere.Center) });
	}
}

void Pu::ConstraintSystem::TestHeightmapSphere(PhysicsHandle hmap, PhysicsHandle hsphere)
{
	/* Get the indices of the objects. */
	const uint16 i = physics_get_lookup_id(hmap);
	const uint16 j = physics_get_lookup_id(hsphere);

	/* Query the colliders and transform them to the correct position. */
	const HeightMap &heightmap = *reinterpret_cast<HeightMap*>(narrowPhases[i]);
	const Vector3 offset = movement->CreateTransform(i).GetTranslation();
	const Sphere sphere = *reinterpret_cast<Sphere*>(narrowPhases[j]) * movement->CreateTransform(j);

	/* Query the heightmap for the height and normal under the sphere. */
	float h;
	Vector3 n;
	if (heightmap.TryGetHeightAndNormal(Vector2(sphere.Center.X - offset.X, sphere.Center.Z - offset.Z), h, n))
	{
		/* The sphere collides with the heightmap is it's lowest point is below the sample height. */
		if (h >= sphere.Center.Y - sphere.Radius)
		{
			solver->RegisterCollision(CollisionManifold{ hmap, hsphere, n });
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
	for (float *narrow : narrowPhases) free(narrow);
}