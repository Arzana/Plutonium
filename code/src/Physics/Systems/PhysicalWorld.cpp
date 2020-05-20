#include "Physics/Systems/PhysicalWorld.h"
#include "Core/Diagnostics/Profiler.h"
#include "Physics/Systems/ShapeTests.h"

/*
Physical handles store various pieces of fast information.
[TT000000 00000000 00000000 00000000 IIIIIIII IIIIIIII IIIIIIII IIIIIIII]

T (2-bits): The type of the object, also the vector in which it is stored.
I (32-bits): The index in the lookup vector, used to determine the actual index.
*/

#define PHYSICS_LIST_PLANE						0ull
#define PHYSICS_LIST_KINEMATIC					2ull

#define PHYSICS_HANDLE_NULL						~0ull
#define PHYSICS_HANDLE_LOOKUP_ID(handle)		((handle) & 0xFFFFFFFF)
#define PHYSICS_HANDLE_TYPE(handle)				((handle) >> 62)
#define PHYSICS_HANDLE_CREATE(type, idx)		(((type) << 62) | (idx))

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

Pu::PhysicsHandle Pu::PhysicalWorld::AddPlane(CollisionPlane && plane)
{
	const PhysicsHandle handle = CreateNewHandle(PHYSICS_LIST_PLANE);
	planes.emplace_back(std::move(plane));
	return handle;
}

Pu::PhysicsHandle Pu::PhysicalWorld::AddKinematic(PhysicalObject && obj)
{
	/* 
	We need to copy over the pointer data in the collider, 
	to make sure it doesn't go out of scope.
	*/
	void *data = nullptr;
	switch (obj.Collider.NarrowPhaseShape)
	{
	case CollisionShapes::None:
		break;
	case CollisionShapes::Sphere:
		data = malloc(sizeof(Sphere));
		memcpy(data, obj.Collider.NarrowPhaseParameters, sizeof(Sphere));
		break;
	default:
		Log::Error("Unable to add kinematic object (invalid narrow phase)!");
		return PHYSICS_HANDLE_NULL;
	}

	obj.Collider.NarrowPhaseParameters = data;
	const PhysicsHandle handle = CreateNewHandle(PHYSICS_LIST_KINEMATIC);
	kinematicObjects.emplace_back(std::move(obj));
	return handle;
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
	internalHandle = PHYSICS_HANDLE_NULL;
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

		Plane plane = planes[idx].Plane;
		return Matrix::CreateLookIn(plane.N * plane.D, tangent(plane.N), plane.N);
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

void Pu::PhysicalWorld::Update(float dt)
{
	Profiler::Begin("Physics", Color::Gray());

	CheckForCollisions();
	//SolveContraints();
	Integrate(dt);

	Profiler::End();
}

void Pu::PhysicalWorld::ThrowInvalidHandle(bool condition, const char *action)
{
	if (condition) Log::Fatal("Cannot %s physics object (invalid handle)!", action);
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
	else if (list == PHYSICS_LIST_KINEMATIC)
	{
#ifdef _DEBUG
		ThrowInvalidHandle(idx >= kinematicObjects.size(), "destroy");
#endif

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
	case PHYSICS_LIST_KINEMATIC:
		i = kinematicObjects.size();
		break;
	default:
		Log::Fatal("Unable to create physics handle (invalid object type)!");
		return PHYSICS_HANDLE_NULL;
	}

	/* Search for an empty position in the lookup table. */
	for (size_t j = 0; j < lookup.size(); j++)
	{
		if (lookup[j] == PHYSICS_HANDLE_NULL)
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
				Log::Warning("Cannot handle collision shape currently!");
				break;
			}
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

void Pu::PhysicalWorld::Integrate(float dt)
{
	for (PhysicalObject &cur : kinematicObjects)
	{
		cur.V += Gravity * dt;
		cur.P += cur.V * dt;
	}
}