#pragma once
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a 3D axis-aligned bounding box. */
	struct AABB
	{
		/* The lower bound of the axis alligned bounding box (position). */
		Vector3 LowerBound;
		/* The upper bound of the axis alligned bounding box (position + size). */
		Vector3 UpperBound;

		/* Initializes an empty instance of a axis alligned bounding box. */
		AABB(void)
		{}

		/* Initializes a new instance of a axis alligned bounding box with a specific size. */
		AABB(_In_ Vector3 size)
			: UpperBound(size)
		{}

		/* Initializes a new instance of an axis alligned bounding box. */
		AABB(_In_ Vector3 lowerBound, _In_ Vector3 upperBound)
			: LowerBound(lowerBound), UpperBound(upperBound)
		{}

		/* Initializes a new instance of a box with a specified origin and size. */
		AABB(_In_ float x, _In_ float y, _In_ float z, _In_ float w, _In_ float h, _In_ float d)
			: LowerBound(x, y, z), UpperBound(x + w, y + h, z + d)
		{}

		/* Transforms the box with the specified matrix. */
		_Check_return_ AABB operator *(_In_ const Matrix &m) const;
		/* Gets the corner at the specified index. */
		_Check_return_ Vector3 operator [](_In_ size_t idx) const;
		/* Adds the specified offset to the AABB. */
		_Check_return_ AABB operator +(_In_ Vector3 offset) const;

		/* Gets whether the box has a size of zero. */
		_Check_return_ inline bool IsEmpty(void) const
		{
			return LowerBound == UpperBound;
		}

		/* Gets whether the box has a size of zero and is at the position origin. */
		_Check_return_ inline bool IsUseless(void) const
		{
			return LowerBound == Vector3() && IsEmpty();
		}

		/* Gets the absolute width of the box. */
		_Check_return_ inline float GetWidth(void) const
		{
			return fabsf(GetSize().X);
		}

		/* Gets the absolute height of the box. */
		_Check_return_ inline float GetHeight(void) const
		{
			return fabsf(GetSize().Y);
		}

		/* Gets the absolute depth of the box. */
		_Check_return_ inline float GetDepth(void) const
		{
			return fabsf(GetSize().Z);
		}

		/* Gets the center point of the box. */
		_Check_return_ inline Vector3 GetCenter(void) const
		{
			return (LowerBound + UpperBound) * 0.5f;
		}

		/* Gets the dimensions of the axis alligned bounding box. */
		_Check_return_ inline Vector3 GetSize(void) const
		{
			return UpperBound - LowerBound;
		}

		/* Gets the surface area of the axis alligned bounding box. */
		_Check_return_ inline float GetArea(void) const
		{
			const Vector3 dim = GetSize();
			return 2.0f * (dim.X * dim.Y + dim.Y * dim.Z + dim.Z * dim.X);
		}

		/* Mixes the two boxes with a specified amount. */
		_Check_return_ static AABB Mix(_In_ const AABB &first, _In_ const AABB &second, _In_ float a);

		/* Expands the box from all faces by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical, _In_ float depth);
		/* Creates a box that contains the two input boxes. */
		_Check_return_ AABB Merge(_In_ const AABB &second) const;
		/* Creates a box that contains the input box and the specified point. */
		_Check_return_ AABB Merge(_In_ Vector3 point) const;
		/* Merges the specified box into this box. */
		void MergeInto(_In_ const AABB &second);
		/* Checks whether a box is fully within the box. */
		_Check_return_ bool Contains(_In_ const AABB &r) const;
		/* Gets the overlap of a box over the box. */
		_Check_return_ AABB GetOverlap(_In_ const AABB &r) const;
		/* Gets the distance from a single point to the box. */
		_Check_return_ float GetDistance(_In_ Vector3 point) const;
	};

	/* Transforms the box with the specified matrix. */
	_Check_return_ inline AABB operator *(_In_ const Matrix &m, _In_ const AABB &b)
	{
		return b * m;
	}
}