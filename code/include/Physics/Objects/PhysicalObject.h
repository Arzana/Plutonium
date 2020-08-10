#pragma once
#include "Collider.h"
#include "Core/Math/Matrix3.h"
#include "Physics/Properties/PhysicalState.h"
#include "PhysicsHandle.h"

namespace Pu
{
	/* Defines a physical object places in the world. */
	class PhysicalObject
	{
	public:
		/* Specifies the world location of the object. */
		Vector3 P;
		/* Specifies the orientation of the object. */
		Quaternion Theta;
		/* Specifies the linear velocity of the object. */
		Vector3 V;
		/* Specifies the angular velocity of the object. */
		Vector3 Omega;
		/* Specified the scale of the object. */
		Vector3 Scale;

		/* Specifies the moment of inertia of the object (not scaled by scale!). */
		Matrix3 MoI;
		/* Specifies the center of mass of the object. */
		Vector3 CoM;
		/* Specifies the current physical state of the object. */
		PhysicalState State;
		/* Specifies the ID of the physical properties of which this physical object is made. */
		PhysicsHandle Properties;
		/* Specifies the collider used by the object. */
		Collider Collider;

		/* Initializes an empty instance of a physical object. */
		PhysicalObject(void)
			: Properties(PhysicsNullHandle), Scale(1.0f)
		{}

		/* Initializes a new instance of a physical object without a collider. */
		PhysicalObject(_In_ Vector3 pos, _In_ Quaternion orien)
			: P(pos), Theta(orien), Scale(1.0f), Properties(PhysicsNullHandle)
		{}

		/* Initializes a new instance of a physical object. */
		PhysicalObject(_In_ Vector3 pos, _In_ Quaternion orien, const Pu::Collider &collider)
			: P(pos), Theta(orien), Scale(1.0f), Properties(PhysicsNullHandle),
			Collider(collider)
		{}
	};
}