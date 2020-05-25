#pragma once
#include "Core/Math/Matrix.h"
#include "Core/Math/Interpolation.h"

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

		/* Expands the box from all faces by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical, _In_ float depth);
		/* Merges the specified point into this box. */
		void MergeInto(_In_ Vector3 p);
		/* Merges the specified box into this box. */
		void MergeInto(_In_ const AABB &second);
	};

	/* Transforms the box with the specified matrix. */
	_Check_return_ inline AABB operator *(_In_ const Matrix &m, _In_ const AABB &b)
	{
		return b * m;
	}

	/* Adds the specified offset to the specified AABB. */
	_Check_return_ inline AABB operator +(_In_ Vector3 offset, _In_ const AABB &box)
	{
		return box + offset;
	}

	/* Performs linear interpolation between the two specified axis-aligned bounding boxes. */
	_Check_return_ inline AABB lerp(_In_ const AABB &a, _In_ const AABB &b, _In_ float v)
	{
		return AABB(lerp(a.LowerBound, b.LowerBound, v), lerp(a.UpperBound, b.UpperBound, v));
	}

	/* Creates an exis-aligned bounding box that contains the input box and point. */
	_Check_return_ inline AABB union_(_In_ const AABB &a, _In_ Vector3 p)
	{
		return AABB(min(a.LowerBound, p), max(a.UpperBound, p));
	}

	/* Creates an axis-aligned bounding box that contains both input boxes. */
	_Check_return_ inline AABB union_(_In_ const AABB &a, _In_ const AABB &b)
	{
		return AABB(min(a.LowerBound, b.LowerBound), max(a.UpperBound, b.UpperBound));
	}
}