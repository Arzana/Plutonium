#include "Physics/Systems/PhysicalWorld.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/HeightMap.h"

using namespace Pu;

uint32 Pu::PhysicalWorld::narrowChecks = 0;

uint32 Pu::PhysicalWorld::GetNarrowCheckCount(void)
{
	return narrowChecks;
}

void Pu::PhysicalWorld::VisualizeCollision(DebugRenderer & renderer, Vector3 camPos) const
{
	Profiler::BeginDebug();
	lock.lock();

	for (const CollisionPlane &collider : planes)
	{
		const Plane &plane = collider.Plane;
		renderer.AddArrow(plane.N * plane.D, plane.N, Color::Green());
	}

	for (const PhysicalObject &obj : staticObjects)
	{
		VisualizeCollider(renderer, obj, Color::Green(), camPos);
	}

	for (const PhysicalObject &obj : kinematicObjects)
	{
		VisualizeCollider(renderer, obj, Color::Blue(), camPos);
	}

	lock.unlock();
	Profiler::End();
}

void Pu::PhysicalWorld::VisualizeBVH(DebugRenderer & renderer) const
{
#ifdef _DEBUG
	lock.lock();
	bvh.Visualize(renderer);
	lock.unlock();
#else 
	(void)renderer;
#endif
}

void Pu::PhysicalWorld::VisualizeCollider(DebugRenderer & renderer, const PhysicalObject & obj, Color clr, Vector3 camPos)
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
	else if (obj.Collider.NarrowPhaseShape == CollisionShapes::HeightMap)
	{
		/* Rendering a heightmap is extremely expensive, so only do it for the closest one. */
		const HeightMap &collider = *reinterpret_cast<HeightMap*>(obj.Collider.NarrowPhaseParameters);
		camPos -= obj.P;

		if (collider.Contains(Vector2(camPos.X, camPos.Z))) collider.Visualize(renderer, obj.P, clr);
		else renderer.AddBox(obj.Collider.BroadPhase + obj.P, clr);
	}
}

void Pu::PhysicalWorld::CheckForCollisions(void)
{
	/* Update the kinematic object AABB if it's out of range. */
	size_t i = 0;
	for (PhysicalObject &obj : kinematicObjects)
	{
		if (sqrdist(obj.P, obj.Q) > sqr(KinematicExpansion))
		{
			const PhysicsHandle hobj = create_physics_handle(PhysicsType::Kinematic, i);
			AABB aabb = obj.Collider.BroadPhase + obj.P;
			aabb.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);

			obj.Q = obj.P;
			bvh.Remove(hobj);
			bvh.Insert(hobj, aabb);
		}

		++i;
	}

	/* Check for plane constraints with kinematic objects. */
	for (i = 0; i < planes.size(); i++)
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

	i = 0;

	/* Check for collision. */
	vector<PhysicsHandle> broadPhaseResult;
	for (const PhysicalObject &second : kinematicObjects)
	{
		/* Traverse the BVH to perform broad phase for this kinematic object. */
		const PhysicsHandle hsecond = create_physics_handle(PhysicsType::Kinematic, i++);
		bvh.Boxcast(second.Collider.BroadPhase + second.P, broadPhaseResult);

		/* Perform narrow phase for all the hits, ignoring self. */
		for (const PhysicsHandle hfirst : broadPhaseResult)
		{
			if (hfirst == hsecond) continue;
			TestGeneric(QueryInternal(hfirst), hfirst, second, hsecond);
		}

		broadPhaseResult.clear();
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
					create_physics_handle(PhysicsType::Plane, planeIdx),
					create_physics_handle(PhysicsType::Kinematic, sphereIdx),
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
			DestroyInternal(create_physics_handle(PhysicsType::Kinematic, sphereIdx));
			return false;
		}
	}

	return true;
}

void Pu::PhysicalWorld::TestGeneric(const PhysicalObject & first, PhysicsHandle hfirst, const PhysicalObject & second, PhysicsHandle hsecond)
{
	++narrowChecks;

	/*
	Construct a key from the collision type, then check if it's in the list.
	If not, try again, but with reverse order.
	Otherwise it's an invalid collision.
	*/
	uint16 key = create_collision_type(first.Collider.NarrowPhaseShape, second.Collider.NarrowPhaseShape);
	decltype(checkers)::iterator it = checkers.find(key);
	if (it != checkers.end())
	{
		((*this).*it->second)(first, hfirst, second, hsecond);
		return;
	}

	key = create_collision_type(second.Collider.NarrowPhaseShape, first.Collider.NarrowPhaseShape);
	it = checkers.find(key);
	if (it != checkers.end())
	{
		((*this).*it->second)(second, hsecond, first, hfirst);
		return;
	}

	Log::Warning("Unable to check for collision between %s and %s!", to_string(first.Collider.NarrowPhaseShape), to_string(second.Collider.NarrowPhaseShape));
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

void Pu::PhysicalWorld::TestHeightMapSphere(const PhysicalObject & first, PhysicsHandle hfirst, const PhysicalObject & second, PhysicsHandle hsecond)
{
	const HeightMap &heightMap = *reinterpret_cast<HeightMap*>(first.Collider.NarrowPhaseParameters);
	Sphere sphere = *reinterpret_cast<Sphere*>(second.Collider.NarrowPhaseParameters);
	sphere.Center += second.P;

	float h;
	Vector3 n;
	if (heightMap.TryGetHeightAndNormal(Vector2(sphere.Center.X - first.P.X, sphere.Center.Z - first.P.Z), h, n))
	{
		/* The sphere collides with the heightmap is it's lowest point is below the sample height. */
		if (h >= sphere.Center.Y - sphere.Radius)
		{
			collisions.emplace_back(CollisionManifold
				{
					hfirst,
					hsecond,
					n
				});
		}
	}
}