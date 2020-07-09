#pragma once
#include <map>
#include "ContactSolverSystem.h"
#include "Physics/Objects/BVH.h"
#include "Physics/Properties/CollisionShapes.h"

namespace Pu
{
	class PhysicalWorld;

	/* Defines a system used to detect collisions. */
	class ContactSystem
	{
	public:
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

		/* Gets the amount of narrow phase checks since the last reset call. */
		_Check_return_ static uint32 GetNarrowPhaseChecks(void);
		/* Resets the narrow phase check counter. */
		static void ResetCounter(void);

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
		void SetGenericCheckers(void);
		void Destroy(void);
	};
}