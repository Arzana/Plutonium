#include "Physics/Systems/ContactSystem.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Physics/Systems/MovementSystem.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/HeightMap.h"

#define collision_t(first, second)	(static_cast<Pu::uint16>(static_cast<Pu::uint16>(first) | static_cast<Pu::uint16>(second) << 8))
#define as_shape(type, params)		(*reinterpret_cast<const Pu::type*>(params))

static Pu::uint32 bvhUpdateCalls = 0;
static Pu::uint32 narrowPhaseChecks = 0;
static Pu::uint32 collisionCount = 0;

Pu::ContactSystem::ContactSystem(PhysicalWorld & world)
	: world(&world), OnTriggerHit("ContactSystemOnTriggerHit")
{
	SetGenericCheckers();
}

Pu::ContactSystem::ContactSystem(ContactSystem && value)
	: OnTriggerHit(std::move(value.OnTriggerHit)),
	checkers(std::move(value.checkers)), world(value.world),
	rawBroadPhase(std::move(value.rawBroadPhase)),
	cachedBroadPhase(std::move(value.cachedBroadPhase)),
	rawNarrowPhase(std::move(value.rawNarrowPhase)),
	hitTriggers(std::move(value.hitTriggers)),
	hfirsts(std::move(value.hfirsts)), hseconds(std::move(value.hseconds)),
	nx(std::move(value.nx)), ny(std::move(value.ny)), nz(std::move(value.nz)),
	px(std::move(value.px)), py(std::move(value.py)), pz(std::move(value.pz)),
	sd(std::move(value.sd)), em(std::move(value.em))
{
	SetGenericCheckers();
}

Pu::ContactSystem & Pu::ContactSystem::operator=(ContactSystem && other)
{
	if (this != &other)
	{
		Destroy();

		OnTriggerHit = std::move(other.OnTriggerHit);
		hfirsts = std::move(other.hfirsts);
		hseconds = std::move(other.hseconds);
		px = std::move(other.px);
		py = std::move(other.py);
		pz = std::move(other.pz);
		nx = std::move(other.nx);
		ny = std::move(other.ny);
		nz = std::move(other.nz);
		sd = std::move(other.sd);
		em = std::move(other.em);

		world = other.world;
		rawBroadPhase = std::move(other.rawBroadPhase);
		cachedBroadPhase = std::move(other.cachedBroadPhase);
		rawNarrowPhase = std::move(other.rawNarrowPhase);
		hitTriggers = std::move(other.hitTriggers);
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
	case CollisionShapes::OBB:
		copy = reinterpret_cast<float*>(malloc(sizeof(OBB)));
		memcpy(copy, collider, sizeof(OBB));
		break;
	case CollisionShapes::HeightMap:
		if (physics_get_type(handle) == PhysicsType::Kinematic)
		{
			Log::Error("Kinematic objects cannot have a HeightMap collider!");
			return;
		}

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
	px.clear();
	py.clear();
	pz.clear();
	nx.clear();
	ny.clear();
	nz.clear();
	sd.clear();
	em.clear();

	if constexpr (ProfileWorldSystems) Profiler::Begin("BVH Update", Color::Abbey());

	/* Query the movement system for updates to the BVH. */
	readdCache.clear();
	world->sysMove->CheckDistance(readdCache);
	for (size_t idx : readdCache)
	{
		/* Remove the old bounding box from the BVH. */
		const PhysicsHandle hobj = world->QueryPublicHandle(create_physics_handle(PhysicsType::Kinematic, idx));
		world->searchTree.Remove(hobj);

		/* Create the new cached bounding box. */
		AABB newBB = rawBroadPhase[idx] * world->GetTransform(hobj);
		newBB.Inflate(KinematicExpansion, KinematicExpansion, KinematicExpansion);

		/* Insert the new bounding box. */
		cachedBroadPhase.at(hobj) = newBB;
		world->searchTree.Insert(hobj, newBB);
		++bvhUpdateCalls;
	}

	if constexpr (ProfileWorldSystems) Profiler::End();

	/* Check for collisions. */
	for (const auto &[hobj, bb] : cachedBroadPhase)
	{
		/* We don't have to check sleeping or static objects. */
		if (physics_get_type(hobj) == PhysicsType::Static || hobj & PhysicsHandleSkipBit) continue;
		if (world->sysMove->IsSleeping(world->QueryInternalIndex(hobj))) continue;

		/* Traverse the BVH to perform broad phase for this kinematic object. */
		if constexpr (ProfileWorldSystems) Profiler::Begin("Broadphase", Color::Crimson());
		broadPhaseCache.clear();
		world->searchTree.Boxcast(bb, broadPhaseCache);

		if constexpr (ProfileWorldSystems)
		{
			Profiler::End();
			Profiler::Begin("Narrowphase", Color::Scarlet());
		}

		/* Perform narrow phase for all the hits, ignoring self. */
		for (const PhysicsHandle hhit : broadPhaseCache)
		{
			if (hhit != hobj && hhit ^ PhysicsHandleSkipBit) TestGeneric(hhit, hobj);
		}

		if constexpr (ProfileWorldSystems) Profiler::End();
	}

#ifdef _DEBUG
	visualizeContacts = false;
#endif
}

void Pu::ContactSystem::ProcessTriggers(void)
{
	for (const auto[hfirst, hsecond] : hitTriggers)
	{
		OnTriggerHit.Post(*this, hfirst, hsecond);
	}

	hitTriggers.clear();
}

#ifdef _DEBUG
void Pu::ContactSystem::VisualizeColliders(DebugRenderer & dbgRenderer, Vector3 camPos) const
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
		else if (narrow.first == CollisionShapes::Sphere) dbgRenderer.AddSphere(as_shape(Sphere, narrow.second) * transform, clr);
		else if (narrow.first == CollisionShapes::OBB) dbgRenderer.AddBox(as_shape(OBB, narrow.second) * transform, clr);
		else if (narrow.first == CollisionShapes::HeightMap)
		{
			const HeightMap &collider = as_shape(HeightMap, narrow.second);
			const Vector3 offset = transform.GetTranslation();

			/* The heightmap is incredibly expensive to debug render, so only do it for one. */
			if (collider.Contains(Vector2(camPos.X - offset.X, camPos.Z - offset.Z))) collider.Visualize(dbgRenderer, offset, clr);
			else dbgRenderer.AddBox(cachedBroadPhase.at(hcur), clr);
		}
	}
}

void Pu::ContactSystem::VisualizeContacts(DebugRenderer & dbgRenderer) const
{
	Profiler::BeginDebug();
	visualizeContacts = true;
	const pu_clock::time_point now = pu_now();

	/* Render all the point that haven't timed out. */
	for (size_t i = 0; i < contacts.size();)
	{
		if (pu_ms(contacts[i].first, now) * 0.001f > PhysicsDebuggingTTL) contacts.removeAt(i);
		else dbgRenderer.AddSphere(contacts[i++].second, 0.1f, Color::Yellow());
	}

	Profiler::End();
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
	decltype(checkers)::iterator it = checkers.find(collision_t(shape1, shape2));
	if (it != checkers.end())
	{
		((*this).*it->second)(hfirst, hsecond);
		return;
	}

	it = checkers.find(collision_t(shape2, shape1));
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
	const Sphere sphere1 = as_shape(Sphere, rawNarrowPhase.at(hfirst).second) * world->GetTransform(hfirst);
	const Sphere sphere2 = as_shape(Sphere, rawNarrowPhase.at(hsecond).second) * world->GetTransform(hsecond);

	const float d2 = sqrdist(sphere1.Center, sphere2.Center);
	const float r2 = sqr(sphere1.Radius + sphere2.Radius);

	/* Check for collision. */
	if (d2 < r2)
	{
		const Vector3 n = dir(sphere1.Center, sphere2.Center);
		const Vector3 p = sphere1.Center + sphere1.Radius * n;
		AddManifold(hfirst, hsecond, p, n, r2 - d2, 1.0f);
	}
}

void Pu::ContactSystem::TestAABBSphere(PhysicsHandle haabb, PhysicsHandle hsphere)
{
	/* Query the sphere collider and transform it to the correct position. */
	const Sphere sphere = as_shape(Sphere, rawNarrowPhase.at(hsphere).second) * world->GetTransform(hsphere);

	const Vector3 q = closest(cachedBroadPhase.at(haabb), sphere.Center);
	const float d2 = sqrdist(q, sphere.Center);
	const float r2 = sqr(sphere.Radius);

	/* Check for collision with the cached bounding box. */
	if (d2 < r2)
	{
		const Vector3 n = dir(q, sphere.Center);
		const Vector3 p = sphere.Center + sphere.Radius * -n;
		AddManifold(haabb, hsphere, p, n, r2 - d2, 1.0f);
	}
}

void Pu::ContactSystem::TestHeightmapSphere(PhysicsHandle hmap, PhysicsHandle hsphere)
{
	/* Query the colliders and transform them to the correct position. */
	const HeightMap &heightmap = as_shape(HeightMap, rawNarrowPhase.at(hmap).second);
	const Vector3 offset = world->GetTransform(hmap).GetTranslation();
	const Sphere sphere = as_shape(Sphere, rawNarrowPhase.at(hsphere).second) * world->GetTransform(hsphere);

	float h;
	Vector3 n;

	/* Query the heightmap for the height and normal under the sphere. */
	const Vector2 query = Vector2(sphere.Center.X - offset.X, sphere.Center.Z - offset.Z);
	if (heightmap.TryGetHeightAndNormal(query, h, n))
	{
		const float low = sphere.Center.Y - sphere.Radius;

		/* The sphere collides with the heightmap is it's lowest point is below the sample height. */
		if (h >= low)
		{
			const Vector3 p{ sphere.Center.X, h, sphere.Center.Z };
			AddManifold(hmap, hsphere, p, n, h - low, 1.0f);
		}
	}
}

void Pu::ContactSystem::TestSphereOBB(PhysicsHandle hsphere, PhysicsHandle hobb)
{
	/* Query the colliders and transform them to the correct location. */
	const Sphere sphere = as_shape(Sphere, rawNarrowPhase.at(hsphere).second) * world->GetTransform(hsphere);
	const OBB &obb = as_shape(OBB, rawNarrowPhase.at(hobb).second) * world->GetTransform(hobb);

	const Vector3 q = closest(obb, sphere.Center);
	const float d2 = sqrdist(q, sphere.Center);
	const float r2 = sqr(sphere.Radius);

	/* Check for collision. */
	if (d2 < r2)
	{
		const Vector3 n = dir(q, sphere.Center);
		const Vector3 p = sphere.Center + sphere.Radius * -n;
		AddManifold(hobb, hsphere, p, n, r2 - d2, 1.0f);
	}
}

void Pu::ContactSystem::TestAABBOBB(PhysicsHandle haabb, PhysicsHandle hobb)
{
	/* Query the colliders and transform them to the correct location. */
	const AABB &aabb = cachedBroadPhase.at(haabb);
	const OBB &obb = as_shape(OBB, rawNarrowPhase.at(hobb).second) * world->GetTransform(hobb);

	/* Check for collision. */
	if (sat.Run(aabb, obb))
	{
		const vector<Vector3> &points = sat.GetContacts(aabb, obb);
		const float mul = 1.0f / points.size();

		/* Should make a different solver for this, but for now this works. */
		for (Vector3 p : points)
		{
			AddManifold(haabb, hobb, p, sat.GetIntersectionAxis(), sat.GetIntersectionDepth(), mul);
		}
	}
}

void Pu::ContactSystem::TestOBBOBB(PhysicsHandle hfirst, PhysicsHandle hsecond)
{
	/* Query the colliders and transform them to the correct location. */
	const OBB &obb1 = as_shape(OBB, rawNarrowPhase.at(hfirst).second) * world->GetTransform(hfirst);
	const OBB &obb2 = as_shape(OBB, rawNarrowPhase.at(hsecond).second) * world->GetTransform(hsecond);

	/* Check for collision. */
	if (sat.Run(obb1, obb2))
	{
		const vector<Vector3> &points = sat.GetContacts(obb1, obb2);
		const float mul = 1.0f / points.size();

		/* Should make a different solver for this, but for now this works. */
		for (Vector3 p : points)
		{
			AddManifold(hfirst, hsecond, p, sat.GetIntersectionAxis(), sat.GetIntersectionDepth(), mul);
		}
	}
}

void Pu::ContactSystem::AddManifold(PhysicsHandle hfirst, PhysicsHandle hsecond, Vector3 pos, Vector3 normal, float depth, float mul)
{
	/* Ignore any duplicate collisions. */
	for (size_t i = 0; i < hfirsts.size(); i++)
	{
		if (hfirsts[i] == hsecond && hseconds[i] == hfirst) return;
	}

	/* Calculate the velocity at the contact point for the kinematic object. */
	PhysicsHandle hinteral = world->QueryInternalHandle(hsecond);
	uint16 i = physics_get_lookup_id(hinteral);
	Vector3 v = world->sysMove->GetVelocity(i);
	Vector3 w = world->sysMove->GetAngularVelocity(i);
	Vector3 r = pos - world->sysMove->GetPosition(hinteral);
	Vector3 relVloc = v + cross(w, r);

	/* Subtract the first velocity from the relative velocity if the second object is also kinematic. */
	if (physics_get_type(hfirst) != PhysicsType::Static)
	{
		hinteral = world->QueryInternalHandle(hfirst);
		i = physics_get_lookup_id(hinteral);
		v = world->sysMove->GetVelocity(i);
		w = world->sysMove->GetAngularVelocity(i);
		r = pos - world->sysMove->GetPosition(hinteral);
		relVloc -= v - cross(w, r);
	}

	/* Ignore the collision if we have a seperating velocity (i.e. moving away from each other). */
	if (dot(relVloc, normal) > 0.0f) return;

	/* Add the event triggers to a specific buffer for later processing. */
	if (hfirst & PhysicsHandleEventBit) hitTriggers.emplace_back(std::make_pair(hfirst, hsecond));
	if (hsecond & PhysicsHandleEventBit) hitTriggers.emplace_back(std::make_pair(hsecond, hfirst));

	hfirsts.emplace_back(hfirst);
	hseconds.emplace_back(hsecond);
	px.push(pos.X);
	py.push(pos.Y);
	pz.push(pos.Z);
	nx.push(normal.X);
	ny.push(normal.Y);
	nz.push(normal.Z);
	sd.push(depth);
	em.push(mul);

#ifdef _DEBUG
	/* Add the collision point to the contacts list (checking for duplicates just slows it down). */
	if (visualizeContacts) contacts.emplace_back(std::make_pair(pu_now(), pos));
#endif

	++collisionCount;
}

void Pu::ContactSystem::SetGenericCheckers(void)
{
	/*
	Current Support:
	' ': Not needed
	'?': Not supported
	'v': Working

	+-----------+------+--------+---------+-----+------+------+-----------+
	|           | AABB | Sphere | Capsule | OBB | Hull | Mesh | HeightMap |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|   AABB    |      |    v   |    ?    |  v  |  ?   |  ?   |           |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|   Sphere  |  v   |    v   |    ?    |  v  |  ?   |  ?   |     v     |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|  Capsule  |  ?   |    ?   |    ?    |  ?  |  ?   |  ?   |     ?     |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|    OBB    |  v   |    v   |    ?    |  v  |  ?   |  ?   |     ?     |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|   Hull    |  ?   |    ?   |    ?    |  ?  |  ?   |  ?   |     ?     |
	+-----------+------+--------+---------+-----+------+------+-----------+
	|   Mesh    |  ?   |    ?   |    ?    |  ?  |  ?   |  ?   |     ?     |
	+-----------+------+--------+---------+-----+------+------+-----------+
	| HeightMap |      |    v   |    ?    |  ?  |  ?   |  ?   |           |
	+-----------+------+--------+---------+-----+------+------+-----------+

	AABB and Heightmap should only be used for static objects, it makes no sense to check for any combination of them.
	*/
	checkers.emplace(collision_t(CollisionShapes::None, CollisionShapes::Sphere), &ContactSystem::TestAABBSphere);
	checkers.emplace(collision_t(CollisionShapes::Sphere, CollisionShapes::Sphere), &ContactSystem::TestSphereSphere);
	checkers.emplace(collision_t(CollisionShapes::HeightMap, CollisionShapes::Sphere), &ContactSystem::TestHeightmapSphere);
	checkers.emplace(collision_t(CollisionShapes::Sphere, CollisionShapes::OBB), &ContactSystem::TestSphereOBB);
	checkers.emplace(collision_t(CollisionShapes::None, CollisionShapes::OBB), &ContactSystem::TestAABBOBB);
	checkers.emplace(collision_t(CollisionShapes::OBB, CollisionShapes::OBB), &ContactSystem::TestOBBOBB);
}

void Pu::ContactSystem::Destroy(void)
{
	for (auto[hcur, narrow] : rawNarrowPhase) free(narrow.second);
}