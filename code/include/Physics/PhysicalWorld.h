#pragma once
#include "System.h"
#include "Colider.h"

namespace Pu
{
	/* Defines an object responsibe for allocating, updating and querying physical objects. */
	class PhysicalWorld
		: public System
	{
	public:
		/* Initializes a new instance of a physical world system. */
		PhysicalWorld(void);
		PhysicalWorld(_In_ const PhysicalWorld&) = delete;
		/* Move constructor. */
		PhysicalWorld(_In_ PhysicalWorld &&value) = default;

		_Check_return_ PhysicalWorld& operator =(_In_ const PhysicalWorld&) = delete;
		/* Move assignment. */
		_Check_return_ PhysicalWorld& operator =(_In_ PhysicalWorld &&other) = default;

		/* Allocates a new collider in this physical world. */
		_Check_return_ ColliderHndl CreateCollider(_In_ const ColliderCreateInfo &info);

	protected:
		/* Updates the physical world contraints. */
		void Update(_In_ float dt) final;

	private:
		/*
		float[4] Sphere (Center, radius)
		float[6] AABB (Lower bound, upper bounds)
		float[5] Capsule (Center, height, radius)
		float[9] OBB (Lower bound, upper bound, control point)
		float *  Hull (All vertices in mesh)
		float *  Mesh (All vertices in mesh)
		*/
	};
}