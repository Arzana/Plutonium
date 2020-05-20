#pragma once
#include "Core/Math/Matrix3.h"

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
		Matrix3 Orientation;

		/* Initializes an empty instance of an oriented bounding box. */
		OBB(void)
		{}

		/* Initializes a new instance of an oriented bounding box. */
		OBB(_In_ Vector3 center, _In_ Vector3 extent, _In_ Matrix3 orientation)
			: Center(center), Extent(extent), Orientation(orientation)
		{}

		/* Gets the right vector from the orientation matrix. */
		_Check_return_ inline Vector3 GetRight(void) const
		{
			return Orientation.GetRight();
		}

		/* Gets the up vector from the orientation matrix. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return Orientation.GetUp();
		}

		/* Gets the forward vector from the orientation matrix. */
		_Check_return_ inline Vector3 GetForward(void) const
		{
			return Orientation.GetForward();
		}

		/* Gets the lower bound of the oriented box. */
		_Check_return_ Vector3 LowerBound(void) const
		{
			return Center - Orientation * (Extent * 0.5f);
		}

		/* Gets the upper bound of the oriented box. */
		_Check_return_ Vector3 UpperBound(void) const
		{
			return Center + Orientation * (Extent * 0.5f);
		}
	};
}