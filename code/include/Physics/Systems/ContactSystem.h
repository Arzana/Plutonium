#pragma once
#include <map>
#include "Core/Math/Shapes/AABB.h"
#include "Core/Collections/simd_vector.h"
#include "Physics/Objects/PhysicsHandle.h"
#include "Physics/Properties/CollisionShapes.h"

namespace Pu
{
	class PhysicalWorld;
	class DebugRenderer;

	/* Defines a system used to detect collisions. */
	class ContactSystem
	{
	public:
		/* Specifies the first handles for the current collisions. */
		vector<PhysicsHandle> hfirsts;
		/* Specifies the second handles for the current collisions. */
		vector<PhysicsHandle> hseconds;
		/* Defines the x-component of the point of collision. */
		avxf_vector px;
		/* Defines the y-component of the point of collision. */
		avxf_vector py;
		/* Defines the z-component of the point of collision. */
		avxf_vector pz;
		/* Defines the x-component of the collision normal. */
		avxf_vector nx;
		/* Defines the y-component of the collision normal. */
		avxf_vector ny;
		/* Defines the z-component of the collision normal. */
		avxf_vector nz;

		/* Initializes a new instance of a constraint system. */
		ContactSystem(_In_ PhysicalWorld &world);
		ContactSystem(_In_ const ContactSystem&) = delete;
		/* Move contructor. */
		ContactSystem(_In_ ContactSystem &&value);
		/* Releases the resources allocated by the constraint system. */
		~ContactSystem(void)
		{
			Destroy();
		}

		_Check_return_ ContactSystem& operator =(_In_ const ContactSystem&) = delete;
		/* Move assignment. */
		_Check_return_ ContactSystem& operator =(_In_ ContactSystem &&other);

		/* Gets the amount of updates the the BVH that occured in the last reset call. */
		_Check_return_ static uint32 GetBVHUpdateCalls(void);
		/* Gets the amount of narrow phase checks since the last reset call. */
		_Check_return_ static uint32 GetNarrowPhaseChecks(void);
		/* Gets the amount of collisions registered since the last reset call. */
		_Check_return_ static uint32 GetCollisionsCount(void);
		/* Resets the profiling counters. */
		static void ResetCounters(void);

		/* Adds a new collider to the constraint system. */
		void AddItem(_In_ PhysicsHandle handle, _In_ const AABB &bb, _In_ CollisionShapes type, _In_ const float *collider);
		/* Removes the specified item from the constraint system. */
		void RemoveItem(_In_ PhysicsHandle handle);
		/* Checks whether any of the kinematic objects have collided with anything in the scene. */
		void Check(void);

#ifdef _DEBUG
		/* Visualizes the colliders in the world. */
		void Visualize(_In_ DebugRenderer &dbgRenderer, _In_ Vector3 camPos) const;
#endif

	private:
		using CollisionChecker_t = void(ContactSystem::*)(PhysicsHandle hfirst, PhysicsHandle hsecond);

		std::map<uint16, CollisionChecker_t> checkers;
		PhysicalWorld *world;

		vector<AABB> rawBroadPhase;
		std::map<PhysicsHandle, std::pair<CollisionShapes, float*>> rawNarrowPhase;

		std::map<PhysicsHandle, AABB> cachedBroadPhase;
		vector<std::pair<size_t, Vector3>> readdCache;
		vector<PhysicsHandle> broadPhaseCache;

		void TestGeneric(PhysicsHandle hfirst, PhysicsHandle hsecond);
		void TestSphereSphere(PhysicsHandle hfirst, PhysicsHandle hsecond);
		void TestAABBSphere(PhysicsHandle haabb, PhysicsHandle hsphere);
		void TestHeightmapSphere(PhysicsHandle hmap, PhysicsHandle hsphere);
		void AddManifold(PhysicsHandle hfirst, PhysicsHandle hsecond, Vector3 pos, Vector3 normal);
		void SetGenericCheckers(void);
		void Destroy(void);
	};
}