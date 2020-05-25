#include "Physics/Systems/PhysicalWorld.h"
#include "Core/Diagnostics/Profiler.h"
#include "Physics/Systems/ShapeTests.h"
#include "Graphics/Diagnostics/DebugRenderer.h"

/*
Physical handles store various pieces of fast information.
[TT000000 00000000 00000000 00000000 IIIIIIII IIIIIIII IIIIIIII IIIIIIII]

T (2-bits): The type of the object, also the vector in which it is stored.
I (32-bits): The index in the lookup vector, used to determine the actual index.
*/

#define PHYSICS_LIST_PLANE						0ull
#define PHYSICS_LIST_STATIC						1ull
#define PHYSICS_LIST_KINEMATIC					2ull

#define PHYSICS_HANDLE_LOOKUP_ID(handle)		((handle) & 0xFFFFFFFF)
#define PHYSICS_HANDLE_TYPE(handle)				((handle) >> 62)
#define PHYSICS_HANDLE_CREATE(type, idx)		(((type) << 62) | (idx))

static Pu::uint32 collisionCount = 0;

Pu::PhysicalWorld::PhysicalWorld(void)
	: System(), Gravity(0.0f, -9.8f, 0.0f)
{}

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
	const PhysicsHandle handle = CreateNewHandle(PHYSICS_LIST_PLANE);
	planes.emplace_back(plane);
	return handle;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddStatic(const PhysicalObject & obj)
{
	return AddInternal(obj, PHYSICS_LIST_STATIC, staticObjects);
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddKinematic(const PhysicalObject & obj)
{
	return AddInternal(obj, PHYSICS_LIST_KINEMATIC, kinematicObjects);
}

size_t Pu::PhysicalWorld::AddMaterial(const PhysicalProperties & properties)
{
	/* Materials cannot be deleted, so this makes returning an index easier. */
	materials.emplace_back(properties);
	return materials.size() - 1;
}

void Pu::PhysicalWorld::Destroy(PhysicsHandle handle)
{
#ifdef _DEBUG
	ThrowInvalidHandle(PHYSICS_HANDLE_LOOKUP_ID(handle) >= lookup.size(), "destroy");
#endif

	/* Get the internal physics handle. */
	PhysicsHandle &internalHandle = lookup[PHYSICS_HANDLE_LOOKUP_ID(handle)];
	const uint32 list = PHYSICS_HANDLE_TYPE(handle);

#ifdef _DEBUG
	ThrowInvalidHandle(list != PHYSICS_HANDLE_TYPE(internalHandle), "destroy");
#endif

	/* Actually destroy the handle and set it to null. */
	DestroyInternal(internalHandle);
	internalHandle = PhysicsNullHandle;
}

Pu::Matrix Pu::PhysicalWorld::GetTransform(PhysicsHandle handle) const
{
#ifdef _DEBUG
	ThrowInvalidHandle(PHYSICS_HANDLE_LOOKUP_ID(handle) >= lookup.size(), "get transform of");
#endif

	/* Get the internal physics handle. */
	const PhysicsHandle internalHandle = lookup[PHYSICS_HANDLE_LOOKUP_ID(handle)];
	const uint32 idx = PHYSICS_HANDLE_LOOKUP_ID(internalHandle);
	const uint32 list = PHYSICS_HANDLE_TYPE(handle);

#ifdef _DEBUG
	ThrowInvalidHandle(list != PHYSICS_HANDLE_TYPE(internalHandle), "get transform of");
#endif

	if (list == PHYSICS_LIST_PLANE)
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
	else if (list == PHYSICS_LIST_KINEMATIC)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= kinematicObjects.size(), "get transform of");
#endif

		const PhysicalObject &obj = kinematicObjects[idx];
		return Matrix::CreateTranslation(obj.P) * Matrix::CreateRotation(obj.Theta);
	}
	else
	{
		ThrowInvalidHandle(true, "get transform of");
		return Matrix{};
	}
}

void Pu::PhysicalWorld::Visualize(DebugRenderer & renderer) const
{
	for (const CollisionPlane &collider : planes)
	{
		const Plane &plane = collider.Plane;
		renderer.AddArrow(plane.N * plane.D, plane.N, Color::Green());
	}

	for (const PhysicalObject &obj : staticObjects)
	{
		VisualizePhysicalObject(renderer, obj, Color::Green());
	}

	for (const PhysicalObject &obj : kinematicObjects)
	{
		VisualizePhysicalObject(renderer, obj, Color::Blue());
	}
}

void Pu::PhysicalWorld::Update(float dt)
{
	Profiler::Begin("Physics", Color::Gray());
	collisionCount = 0;

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

	Profiler::End();
}

void Pu::PhysicalWorld::ThrowInvalidHandle(bool condition, const char *action)
{
	if (condition) Log::Fatal("Cannot %s physics object (invalid handle)!", action);
}

void Pu::PhysicalWorld::VisualizePhysicalObject(DebugRenderer & renderer, const PhysicalObject & obj, Color clr)
{
	if (obj.Collider.NarrowPhaseShape == CollisionShapes::None)
	{
		renderer.AddBox(obj.Collider.BroadPhase + obj.P, clr);
	}
	else if (obj.Collider.NarrowPhaseShape == CollisionShapes::Sphere)
	{
		const Sphere collider = *reinterpret_cast<Sphere*>(obj.Collider.NarrowPhaseParameters);
		renderer.AddSphere(obj.P + collider.Center, collider.Radius, clr);
	}
	else Log::Warning("Cannot visualize %s yet!", to_string(obj.Collider.NarrowPhaseShape));
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddInternal(const PhysicalObject & obj, uint64 type, vector<PhysicalObject> & list)
{
	/* We need to copy over some collider parameters. */
	PhysicalObject copy = obj;
	switch (obj.Collider.NarrowPhaseShape)
	{
	case CollisionShapes::None:
		if (type == PHYSICS_LIST_KINEMATIC)
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
	default:
		Log::Error("Unable to add object (invalid narrow phase)!");
		return PhysicsNullHandle;
	}

	/* All went well, so create a new handle and add it to the correct list. */
	const PhysicsHandle handle = CreateNewHandle(type);
	list.emplace_back(copy);
	return handle;
}

void Pu::PhysicalWorld::DestroyInternal(PhysicsHandle internalHandle)
{
	const uint32 idx = PHYSICS_HANDLE_LOOKUP_ID(internalHandle);
	const uint32 list = PHYSICS_HANDLE_TYPE(internalHandle);

	/* Destroy the underlying object. */
	if (list == PHYSICS_LIST_PLANE)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= planes.size(), "destroy");
#endif

		planes.removeAt(idx);
	}
	else if (list == PHYSICS_LIST_STATIC)
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
	else if (list == PHYSICS_LIST_KINEMATIC)
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
		const uint64 curList = PHYSICS_HANDLE_TYPE(cur);
		if (curList == list)
		{
			const uint32 curIdx = PHYSICS_HANDLE_LOOKUP_ID(cur);
			if (curIdx > idx) cur = PHYSICS_HANDLE_CREATE(curList, curIdx - 1);
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
Pu::PhysicsHandle Pu::PhysicalWorld::CreateNewHandle(uint64 type)
{
	/* Get the index of the new internal handle in the specified physics list. */
	size_t i;
	switch (type)
	{
	case PHYSICS_LIST_PLANE:
		i = planes.size();
		break;
	case PHYSICS_LIST_STATIC:
		i = staticObjects.size();
		break;
	case PHYSICS_LIST_KINEMATIC:
		i = kinematicObjects.size();
		break;
	default:
		Log::Fatal("Unable to create physics handle (invalid object type)!");
		return PhysicsNullHandle;
	}

	/* Search for an empty position in the lookup table. */
	for (size_t j = 0; j < lookup.size(); j++)
	{
		if (lookup[j] == PhysicsNullHandle)
		{
			lookup[j] = PHYSICS_HANDLE_CREATE(type, i);
			return PHYSICS_HANDLE_CREATE(type, j);
		}
	}

	/* No null entry was found, so just add a new one. */
	lookup.emplace_back(PHYSICS_HANDLE_CREATE(type, i));
	return PHYSICS_HANDLE_CREATE(type, lookup.size() - 1);
}

void Pu::PhysicalWorld::CheckForCollisions(void)
{
	/* Check for plane constraints with kinematic objects. */
	for (size_t i = 0; i < planes.size(); i++)
	{
		for (size_t j = 0; j < kinematicObjects.size();)
		{
			switch (kinematicObjects[j].Collider.NarrowPhaseShape)
			{
			case CollisionShapes::Sphere:
				j += TestPlaneSphere(i, j);
				break;
			default:
				Log::Warning("Cannot handle %s collision shape currently!", to_string(kinematicObjects[j].Collider.NarrowPhaseShape));
				break;
			}
		}
	}

	/* Check for collision with static objects. */
	for (size_t i = 0; i < staticObjects.size(); i++)
	{
		for (size_t j = 0; j < kinematicObjects.size(); j++)
		{
			if (staticObjects[i].Collider.NarrowPhaseShape == CollisionShapes::None)
			{
				if (kinematicObjects[j].Collider.NarrowPhaseShape == CollisionShapes::Sphere)
				{
					TestAABBSphere(staticObjects[i], PHYSICS_HANDLE_CREATE(PHYSICS_LIST_STATIC, i), kinematicObjects[j], PHYSICS_HANDLE_CREATE(PHYSICS_LIST_KINEMATIC, j));
				}
				else Log::Warning("Cannot handle %s collision shape currently!", to_string(kinematicObjects[j].Collider.NarrowPhaseShape));
			}
			else Log::Warning("Cannot handle %s collision shape currently!", to_string(staticObjects[i].Collider.NarrowPhaseShape));
		}
	}

	/* Check for collision with kinematic objects. */
	for (size_t i = 0; i < kinematicObjects.size(); i++)
	{
		for (size_t j = 0; j < kinematicObjects.size(); j++)
		{
			if (i == j) continue;

			if (kinematicObjects[i].Collider.NarrowPhaseShape == CollisionShapes::Sphere)
			{
				if (kinematicObjects[j].Collider.NarrowPhaseShape == CollisionShapes::Sphere)
				{
					TestSphereSphere(kinematicObjects[i], PHYSICS_HANDLE_CREATE(PHYSICS_LIST_KINEMATIC, i), kinematicObjects[j], PHYSICS_HANDLE_CREATE(PHYSICS_LIST_KINEMATIC, j));
				}
				else Log::Warning("Cannot handle %s collision shape currently!", to_string(kinematicObjects[j].Collider.NarrowPhaseShape));
			}
			else Log::Warning("Cannot handle %s collision shape currently!", to_string(kinematicObjects[i].Collider.NarrowPhaseShape));
		}
	}
}

bool Pu::PhysicalWorld::TestPlaneSphere(size_t planeIdx, size_t sphereIdx)
{
	CollisionPlane &plane = planes[planeIdx];
	PhysicalObject &sphere = kinematicObjects[sphereIdx];

	/* Move the collider into world space. */
	Sphere worldSphere = *reinterpret_cast<Sphere*>(sphere.Collider.NarrowPhaseParameters);
	worldSphere.Center += sphere.P;

	/* Do the collision check. */
	if (intersects(worldSphere, plane.Plane))
	{
		/* Add a collision manifold if specified. */
		if (_CrtEnumCheckFlag(plane.Type, PassOptions::KinematicResponse))
		{
			collisions.emplace_back(CollisionManifold
				{
					PHYSICS_HANDLE_CREATE(PHYSICS_LIST_PLANE, planeIdx),
					PHYSICS_HANDLE_CREATE(PHYSICS_LIST_KINEMATIC, sphereIdx),
					plane.Plane.N
				});
		}

		/* Check if a custom event should hit. */
		if (_CrtEnumCheckFlag(plane.Type, PassOptions::Event))
		{
			plane.OnPass.Post(plane, sphere.Collider.UserParam);
		}

		/* Check whether the sphere should be destroyed. */
		if (_CrtEnumCheckFlag(plane.Type, PassOptions::Destroy))
		{
			DestroyInternal(PHYSICS_HANDLE_CREATE(PHYSICS_LIST_KINEMATIC, sphereIdx));
			return false;
		}
	}

	return true;
}

void Pu::PhysicalWorld::TestAABBSphere(const PhysicalObject & first, PhysicsHandle hfirst, const PhysicalObject & second, PhysicsHandle hsecond)
{
	AABB aabb = first.Collider.BroadPhase;
	Sphere sphere = *reinterpret_cast<Sphere*>(second.Collider.NarrowPhaseParameters);

	aabb.LowerBound += first.P;
	aabb.UpperBound += first.P;
	sphere.Center += second.P;

	/* Add collision manifold if the sphere and the AABB overlap. */
	const Vector3 q = closest(aabb, sphere.Center);
	if (sqrdist(q, sphere.Center) < sqr(sphere.Radius))
	{
		collisions.emplace_back(CollisionManifold
			{
				hfirst,
				hsecond,
				dir(q, sphere.Center)
			});
	}
}

void Pu::PhysicalWorld::TestSphereSphere(const PhysicalObject & first, PhysicsHandle hfirst, const PhysicalObject & second, PhysicsHandle hsecond)
{
	Sphere firstSphere = *reinterpret_cast<Sphere*>(first.Collider.NarrowPhaseParameters);
	Sphere secondSphere = *reinterpret_cast<Sphere*>(second.Collider.NarrowPhaseParameters);

	firstSphere.Center += first.P;
	secondSphere.Center += second.P;

	/* Add the collision manifold if the spheres overlap. */
	if (intersects(firstSphere, secondSphere))
	{
		collisions.emplace_back(CollisionManifold
			{
				hfirst,
				hsecond,
				dir(firstSphere.Center, secondSphere.Center)
			});
	}
}

void Pu::PhysicalWorld::SolveContraints(void)
{
	for (const CollisionManifold &manifold : collisions)
	{
		const uint32 at = PHYSICS_HANDLE_TYPE(manifold.FirstObject);
		const uint32 bt = PHYSICS_HANDLE_TYPE(manifold.SecondObject);

		/* Collision with a static object or plane will always store the static object as the first object. */
		if ((at == PHYSICS_LIST_STATIC || at == PHYSICS_LIST_PLANE) && bt == PHYSICS_LIST_KINEMATIC)
		{
			PhysicalObject &second = kinematicObjects[PHYSICS_HANDLE_LOOKUP_ID(manifold.SecondObject)];

			const float e = materials[second.Properties].Mechanical.CoR;
			const float j = rectify(-(1.0f + e) * dot(second.V, manifold.N));
			second.V += j * manifold.N;
		}
		else if (at == PHYSICS_LIST_KINEMATIC && bt == PHYSICS_LIST_KINEMATIC)
		{
			PhysicalObject &first = kinematicObjects[PHYSICS_HANDLE_LOOKUP_ID(manifold.FirstObject)];
			PhysicalObject &second = kinematicObjects[PHYSICS_HANDLE_LOOKUP_ID(manifold.SecondObject)];

			const float imassFirst = recip(first.State.Mass);
			const float imassSecond = recip(second.State.Mass);
			const Vector3 relVloc = second.V - first.V;

			/* Objects are moving away from each other, don't solve collision again. */
			if (dot(relVloc, manifold.N) > 0.0f) continue;

			/* Calculate impulse. */
			const float e = min(materials[first.Properties].Mechanical.CoR, materials[second.Properties].Mechanical.CoR);
			const float j = (-(1.0f + e) * dot(relVloc, manifold.N)) / (imassFirst + imassSecond);

			/* Apply linear impulse. */
			first.V -= j * manifold.N * imassFirst;
			second.V += j * manifold.N * imassSecond;
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