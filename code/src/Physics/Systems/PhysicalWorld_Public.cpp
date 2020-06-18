#include "Physics/Systems/PhysicalWorld.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/HeightMap.h"
#include "Graphics/Diagnostics/DebugRenderer.h"

static Pu::uint32 collisionCount = 0;

Pu::PhysicalWorld::PhysicalWorld(void)
	: System(), Gravity(0.0f, -9.8f, 0.0f)
{
	checkers.emplace(create_collision_type(CollisionShapes::None, CollisionShapes::Sphere), &PhysicalWorld::TestAABBSphere);
	checkers.emplace(create_collision_type(CollisionShapes::Sphere, CollisionShapes::Sphere), &PhysicalWorld::TestSphereSphere);
	checkers.emplace(create_collision_type(CollisionShapes::HeightMap, CollisionShapes::Sphere), &PhysicalWorld::TestHeightMapSphere);
}

Pu::PhysicalWorld::~PhysicalWorld(void)
{
	/* These are allocated by us. */
	for (const PhysicalObject &cur : kinematicObjects)
	{
		free(cur.Collider.NarrowPhaseParameters);
	}
}

Pu::uint32 Pu::PhysicalWorld::GetCollisionCount(void)
{
	return collisionCount;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddPlane(const CollisionPlane & plane)
{
	lock.lock();
	const PhysicsHandle handle = CreateNewHandle(PhysicsType::Plane);
	planes.emplace_back(plane);
	lock.unlock();
	return handle;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddStatic(const PhysicalObject & obj)
{
	return AddInternal(obj, PhysicsType::Static, staticObjects);
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddKinematic(const PhysicalObject & obj)
{
	return AddInternal(obj, PhysicsType::Kinematic, kinematicObjects);
}

size_t Pu::PhysicalWorld::AddMaterial(const PhysicalProperties & properties)
{
	/* Materials cannot be deleted, so this makes returning an index easier. */
	lock.lock();
	materials.emplace_back(properties);
	const size_t result = materials.size() - 1;
	lock.unlock();
	return result;
}

void Pu::PhysicalWorld::Destroy(PhysicsHandle handle)
{
	lock.lock();

#ifdef _DEBUG
	ThrowInvalidHandle(physics_get_lookup_id(handle) >= lookup.size(), "destroy");
#endif

	/* Get the internal physics handle. */
	PhysicsHandle &internalHandle = lookup[physics_get_lookup_id(handle)];
	const PhysicsType list = physics_get_type(handle);

#ifdef _DEBUG
	ThrowInvalidHandle(list != physics_get_type(internalHandle), "destroy");
#endif

	/* Actually destroy the handle and set it to null. */
	DestroyInternal(internalHandle);
	internalHandle = PhysicsNullHandle;

	lock.unlock();
}

Pu::Matrix Pu::PhysicalWorld::GetTransform(PhysicsHandle handle) const
{
#ifdef _DEBUG
	ThrowInvalidHandle(physics_get_lookup_id(handle) >= lookup.size(), "get transform of");
#endif

	/* Get the internal physics handle. */
	const PhysicsHandle internalHandle = lookup[physics_get_lookup_id(handle)];
	const uint16 idx = physics_get_lookup_id(internalHandle);
	const PhysicsType list = physics_get_type(handle);

#ifdef _DEBUG
	ThrowInvalidHandle(list != physics_get_type(internalHandle), "get transform of");
#endif

	if (list == PhysicsType::Plane)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= planes.size(), "get transform of");
#endif
		/* 
		The default state of the plane is N = UP.
		So we just get the tangent of that normal for the right vector,
		and then N X R = FORWARD, translation is simply N * D.
		*/
		const Plane plane = planes[idx].Plane;
		const Vector3 t = plane.N * plane.D;
		const Vector3 r = cross(plane.N, tangent(plane.N));
		const Vector3 f = cross(plane.N, r);
		return Matrix(
			r.X, plane.N.X, f.X, t.X,
			r.Y, plane.N.Y, f.Y, t.Y,
			r.Z, plane.N.Z, f.Z, t.Z,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (list == PhysicsType::Kinematic)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= kinematicObjects.size(), "get transform of");
#endif

		const PhysicalObject &obj = kinematicObjects[idx];
		return Matrix::CreateTranslation(obj.P) * Matrix::CreateRotation(obj.Theta);
	}
	else if (list == PhysicsType::Static)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= staticObjects.size(), "get transform of");

		static bool shouldThrow = true;
		if (shouldThrow)
		{
			shouldThrow = false;
			Log::Warning("Attempting to query transform of static object 0x%X, this is slow, cache it instead (this warning only displays once)!");
		}
#endif

		const PhysicalObject &obj = staticObjects[idx];
		return Matrix::CreateTranslation(obj.P) * Matrix::CreateRotation(obj.Theta);
	}
	else
	{
		ThrowInvalidHandle(true, "get transform of");
		return Matrix{};
	}
}

void Pu::PhysicalWorld::Update(float dt)
{
	Profiler::Begin("Physics", Color::Gray());
	lock.lock();

	collisionCount = 0;
	narrowChecks = 0;

	/*
	We first check for collision events between all objects.
	This should be done early on, but it could be swapped with the next step.

	We add the constant world forces (gravity and dynamics).
	This needs to happen before we solve the collisions as they take
	the final velocity into account when applying their impulses.
	Otherwise the objects would slowely fall through floors, etc.

	Finally we solve for the collision and integrate our positions to the next timestep.
	Thus creating the new state of the world.
	*/
	CheckForCollisions();
	collisionCount = static_cast<uint32>(collisions.size());
	AddConstantForces(dt);
	SolveContraints();
	Integrate(dt);

	lock.unlock();
	Profiler::End();
}

void Pu::PhysicalWorld::ThrowInvalidHandle(bool condition, const char *action)
{
	if (condition) Log::Fatal("Cannot %s physics object (invalid handle)!", action);
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddInternal(const PhysicalObject & obj, PhysicsType type, vector<PhysicalObject> & list)
{
	lock.lock();

	/* We need to copy over some collider parameters. */
	PhysicalObject copy = obj;
	switch (obj.Collider.NarrowPhaseShape)
	{
	case CollisionShapes::None:
		if (type == PhysicsType::Kinematic)
		{
			/*
			Kinematic objects can move throughout the world.
			Therefor only an AABB collider would create very weird physics.
			*/
			Log::Error("Kinematic objects cannot have only an AABB collider!");
			return PhysicsNullHandle;
		}
		break;
	case CollisionShapes::Sphere:
		copy.Collider.NarrowPhaseParameters = malloc(sizeof(Sphere));
		memcpy(copy.Collider.NarrowPhaseParameters, obj.Collider.NarrowPhaseParameters, sizeof(Sphere));
		break;
	case CollisionShapes::HeightMap:
		copy.Collider.NarrowPhaseParameters = malloc(sizeof(HeightMap));
		copy.Collider.NarrowPhaseParameters = new (copy.Collider.NarrowPhaseParameters) HeightMap(*reinterpret_cast<const HeightMap*>(obj.Collider.NarrowPhaseParameters));
		break;
	default:
		Log::Error("Unable to add object (cannot handle %s narrow phase)!", obj.Collider.NarrowPhaseShape);
		return PhysicsNullHandle;
	}

	/* All went well, so create a new handle and add it to the correct list. */
	const PhysicsHandle handle = CreateNewHandle(type);
	list.emplace_back(copy);
	bvh.Insert(lookup[physics_get_lookup_id(handle)], copy.Collider.BroadPhase + copy.P);
	lock.unlock();

	return handle;
}

void Pu::PhysicalWorld::DestroyInternal(PhysicsHandle internalHandle)
{
	const uint16 idx = physics_get_lookup_id(internalHandle);
	const PhysicsType list = physics_get_type(internalHandle);

	if (list != PhysicsType::Plane) bvh.Remove(internalHandle);

	/* Destroy the underlying object. */
	if (list == PhysicsType::Plane)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= planes.size(), "destroy");
#endif

		planes.removeAt(idx);
	}
	else if (list == PhysicsType::Static)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= staticObjects.size(), "destroy");
#endif

		if (staticObjects[idx].Collider.NarrowPhaseShape != CollisionShapes::None)
		{
			free(staticObjects[idx].Collider.NarrowPhaseParameters);
		}

		staticObjects.removeAt(idx);
	}
	else if (list == PhysicsType::Kinematic)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= kinematicObjects.size(), "destroy");
#endif

		/* Narrow phase cannot be null for kinematic objects. */
		free(kinematicObjects[idx].Collider.NarrowPhaseParameters);
		kinematicObjects.removeAt(idx);
	}

	/* Update all the other underlying handles in the list. */
	for (PhysicsHandle &cur : lookup)
	{
		const PhysicsType curList = physics_get_type(cur);
		if (curList == list)
		{
			const uint16 curIdx = physics_get_lookup_id(cur);
			if (curIdx > idx) cur = create_physics_handle(curList, static_cast<uint16>(curIdx - 1u));
		}
	}
}

/*
The lookup table is our way of querying the actual physics objects from the lists.
The goal is to be able to resize the underlying vectors without the handle needing to change.

When an object is added we search through the lookup table for an unused spot (indicated by ~0).
If we find one we'll use that index for this object, otherwise we just emplace a new one.

When an object is removed we must go through the entire lookup table to update the indices of the objects.
*/
Pu::PhysicsHandle Pu::PhysicalWorld::CreateNewHandle(PhysicsType type)
{
	/* Get the index of the new internal handle in the specified physics list. */
	size_t i;
	switch (type)
	{
	case PhysicsType::Plane:
		i = planes.size();
		break;
	case PhysicsType::Static:
		i = staticObjects.size();
		break;
	case PhysicsType::Kinematic:
		i = kinematicObjects.size();
		break;
	default:
		Log::Fatal("Unable to create physics handle (invalid object type)!");
		return PhysicsNullHandle;
	}

#ifdef _DEBUG
	if (i > maxv<uint16>())
	{
		Log::Fatal("Unable to create physics handle (out of space)!");
		return PhysicsNullHandle;
	}
#endif

	/* Search for an empty position in the lookup table. */
	for (size_t j = 0; j < lookup.size(); j++)
	{
		if (lookup[j] == PhysicsNullHandle)
		{
			lookup[j] = create_physics_handle(type, i);
			return create_physics_handle(type, j);
		}
	}

	/* No null entry was found, so just add a new one. */
	lookup.emplace_back(create_physics_handle(type, i));
	return create_physics_handle(type, lookup.size() - 1);
}

Pu::PhysicalObject & Pu::PhysicalWorld::QueryInternal(PhysicsHandle handle)
{
	const uint16 idx = physics_get_lookup_id(handle);
	const PhysicsType list = physics_get_type(handle);

	if (list == PhysicsType::Static) return staticObjects[idx];
	else return kinematicObjects[idx];
}

void Pu::PhysicalWorld::SolveContraints(void)
{
	for (const CollisionManifold &manifold : collisions)
	{
		const PhysicsType at = physics_get_type(manifold.FirstObject);
		const PhysicsType bt = physics_get_type(manifold.SecondObject);

		/* Collision with a static object or plane will always store the static object as the first object. */
		if ((at == PhysicsType::Static || at == PhysicsType::Plane) && bt == PhysicsType::Kinematic)
		{
			PhysicalObject &second = kinematicObjects[physics_get_lookup_id(manifold.SecondObject)];
			const Vector3 t = normalize(second.V - dot(second.V, manifold.N) * manifold.N);

			/* Apply linear impulse. */
			float e = materials[second.Properties].Mechanical.CoR;
			float j = rectify(-(1.0f + e) * dot(second.V, manifold.N));
			second.V += j * manifold.N;

			/* Apply friction. */
			e = materials[second.Properties].Mechanical.CoF;
			j = clamp(-(1.0f + e) * dot(second.V, t), -j * e, j * e);
			second.V += j * t;
		}
		else if (at == PhysicsType::Kinematic && bt == PhysicsType::Kinematic)
		{
			PhysicalObject &first = kinematicObjects[physics_get_lookup_id(manifold.FirstObject)];
			PhysicalObject &second = kinematicObjects[physics_get_lookup_id(manifold.SecondObject)];

			PhysicalProperties &pfirst = materials[first.Properties];
			PhysicalProperties &psecond = materials[second.Properties];

			const float imassFirst = recip(first.State.Mass);
			const float imassSecond = recip(second.State.Mass);
			const float imassTotal = imassFirst + imassSecond;
			const Vector3 relVloc = second.V - first.V;
			const Vector3 t = normalize(relVloc - dot(relVloc, manifold.N) * manifold.N);

			/* Objects are moving away from each other, don't solve collision again. */
			if (dot(relVloc, manifold.N) > 0.0f) continue;

			/* Calculate linear impulse. */
			float e = min(pfirst.Mechanical.CoR, psecond.Mechanical.CoR);
			float j = (-(1.0f + e) * dot(relVloc, manifold.N)) / imassTotal;

			/* Apply linear impulse. */
			first.V -= j * manifold.N * imassFirst;
			second.V += j * manifold.N * imassSecond;

			/* Calculate friction. */
			e = sqrtf(pfirst.Mechanical.CoF * psecond.Mechanical.CoF);
			e = clamp(-(1.0f + e) * dot(relVloc, t) / imassTotal, -j * e, j * e);
			
			/* Apply friction. */
			first.V -= j * t * imassFirst;
			second.V += j * t * imassSecond;
		}
		else Log::Error("Unable to solve constraint for type %u (%x) with type %u (%x)!", at, manifold.FirstObject, bt, manifold.SecondObject);
	}

	collisions.clear();
}

void Pu::PhysicalWorld::AddConstantForces(float dt)
{
	for (PhysicalObject &cur : kinematicObjects)
	{
		/* Add gravity. */
		cur.V += Gravity * dt;

		/* Add aerodynamic drag. */
		const float l = cur.V.LengthSquared();
		const Vector3 f = (cur.V / sqrtf(l)) * cur.State.Cd * l;
		cur.V -= f * recip(cur.State.Mass) * dt;
	}
}

void Pu::PhysicalWorld::Integrate(float dt)
{
	for (PhysicalObject &cur : kinematicObjects)
	{
		cur.P += cur.V * dt;
	}
}