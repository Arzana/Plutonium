#pragma once
#include "System.h"
#include "Physics/Objects/PhysicalObject.h"
#include "Physics/Objects/CollisionPlane.h"

namespace Pu
{
	/* Defines an object responsibe for allocating, updating and querying physical objects. */
	class PhysicalWorld final
		: public System
	{
	public:
		/* Specifies the gravitational force acting upon all kinematic objects. */
		Vector3 Gravity;

		/* Initializes a new instance of a physical world system. */
		PhysicalWorld(void);
		PhysicalWorld(_In_ const PhysicalWorld&) = delete;
		PhysicalWorld(_In_ PhysicalWorld&&) = delete;
		/* Releases the resources allocated by the physical world. */
		virtual ~PhysicalWorld(void);

		_Check_return_ PhysicalWorld& operator =(_In_ const PhysicalWorld&) = delete;
		_Check_return_ PhysicalWorld& operator =(_In_ PhysicalWorld&&) = delete;

		/* Adds the specified collision plane to this world. */
		_Check_return_ PhysicsHandle AddPlane(_In_ CollisionPlane &&plane);
		/* Adds the specified object to this world. */
		_Check_return_ PhysicsHandle AddKinematic(_In_ PhysicalObject &&obj);
		/* Removes the specified physics object from this world. */
		void Destroy(_In_ PhysicsHandle handle);
		/* Gets the transform of the specified physical object. */
		_Check_return_ Matrix GetTransform(_In_ PhysicsHandle handle) const;

	protected:
		/* Updates the physical world contraints. */
		void Update(_In_ float dt) final;

	private:
		vector<CollisionPlane> planes;
		vector<PhysicalObject> staticObjects;
		vector<PhysicalObject> kinematicObjects;

		vector<PhysicsHandle> lookup;
		vector<CollisionManifold> collisions;

		static void ThrowInvalidHandle(bool condition, const char *action);
		void DestroyInternal(PhysicsHandle internalHandle);
		PhysicsHandle CreateNewHandle(uint64 type);
		void CheckForCollisions(void);
		bool TestPlaneSphere(size_t planeIdx, size_t sphereIdx);
		//void SolveContraints(void);
		void Integrate(float dt);
	};
}