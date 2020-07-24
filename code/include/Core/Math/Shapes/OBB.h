#pragma once
#include "AABB.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a 3D oriented bounding box. */
	struct OBB
	{
	public:
		/* Defines the center of the oriented box. */
		Vector3 Center;
		/* Defines half of the size of the oriented box. */
		Vector3 Extent;
		/* Defines the orientation of the box. */
		Quaternion Orientation;

		/* Initializes an empty instance of an oriented bounding box. */
		OBB(void) = default;

		/* Initializes a new instance of an oriented bounding box. */
		OBB(_In_ Vector3 center, _In_ Vector3 extent, _In_ Quaternion orientation)
			: Center(center), Extent(extent), Orientation(orientation)
		{}

		/* Applies the specified transform to this OBB and returns the result. */
		_Check_return_ OBB operator *(_In_ const Matrix &transform) const;
		/* Calculates the bounding box for this OBB. */
		_Check_return_ AABB GetBoundingBox(void) const;

		/* Gets the X-axis of this OBB. */
		_Check_return_ Vector3 GetRight(void) const
		{
			return Orientation * Vector3::Right();
		}
		
		/* Gets the Y-axis of this OBB. */
		_Check_return_ Vector3 GetUp(void) const
		{
			return Orientation * Vector3::Up();
		}

		/* Gets the Z-axis of this OBB. */
		_Check_return_ Vector3 GetForward(void) const
		{
			return Orientation * Vector3::Forward();
		}
	};
}