#pragma once
#include "System.h"
#include "Physics/Objects/PhysicalObject.h"
#include "Physics/Objects/CollisionPlane.h"
#include "Physics/Properties/PhysicalProperties.h"

namespace Pu
{
	class DebugRenderer;
	struct Color;

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

		/* Gets the total amount of collisions since the last reset call. */
		_Check_return_ static uint32 GetCollisionCount(void);
		/* Adds the specified collision plane to this world. */
		_Check_return_ PhysicsHandle AddPlane(_In_ const CollisionPlane &plane);
		/* Adds the specified object to this world. */
		_Check_return_ PhysicsHandle AddKinematic(_In_ const PhysicalObject &obj);
		/* Adds a specific physical material to this world. */
		_Check_return_ size_t AddMaterial(_In_ const PhysicalProperties &properties);
		/* Removes the specified physics object from this world. */
		void Destroy(_In_ PhysicsHandle handle);
		/* Gets the transform of the specified physical object. */
		_Check_return_ Matrix GetTransform(_In_ PhysicsHandle handle) const;
		/* Renders the entire physical world to the specified debug renderer. */
		void Visualize(_In_ DebugRenderer &renderer) const;

	protected:
		/* Updates the physical world contraints. */
		void Update(_In_ float dt) final;

	private:
		vector<CollisionPlane> planes;
		vector<PhysicalObject> staticObjects;
		vector<PhysicalObject> kinematicObjects;
		vector<PhysicalProperties> materials;

		vector<PhysicsHandle> lookup;
		vector<CollisionManifold> collisions;

		static void ThrowInvalidHandle(bool condition, const char *action);
		static void VisualizePhysicalObject(DebugRenderer &renderer, const PhysicalObject &obj, Color clr);

		void DestroyInternal(PhysicsHandle internalHandle);
		PhysicsHandle CreateNewHandle(uint64 type);
		void CheckForCollisions(void);
		bool TestPlaneSphere(size_t planeIdx, size_t sphereIdx);
		void TestSphereSphere(const PhysicalObject &first, PhysicsHandle hfirst, const PhysicalObject &second, PhysicsHandle hsecond);
		void SolveContraints(void);
		void SolvePlaneContraint(size_t planeIdx, size_t kinematicIdx);
		void AddConstantForces(float dt);
		void Integrate(float dt);
	};
}