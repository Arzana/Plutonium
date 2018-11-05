#pragma once
#include "Matrix.h"

namespace Pu
{
	/* Defines a 3D axis alligned bounding box. */
	struct AABB
	{
		/* The origin of the box. */
		Vector3 Position;
		/* The size of the box. */
		Vector3 Size;

		/* Initializes a new instance of an empty box. */
		AABB(void)
			: Position(), Size()
		{}

		/* Initializes a new instance of a box with a specified size. */
		AABB(_In_ Vector3 size)
			: Position(), Size(size)
		{}

		/* Initializes a new instance of a box with a specified orinin and size. */
		AABB(_In_ Vector3 pos, _In_ Vector3 size)
			: Position(pos), Size(size)
		{}

		/* Initializes a new instance of a box with a specified origin and size. */
		AABB(_In_ float x, _In_ float y, _In_ float z, _In_ float w, _In_ float h, _In_ float d)
			: Position(x, y, z), Size(w, h, d)
		{}

		/* Checks whether the two boxes are equal. */
		_Check_return_ inline bool operator ==(_In_ const AABB &b) const
		{
			return Position == b.Position && Size == b.Size;
		}

		/* Checks whether the two boxes differ. */
		_Check_return_ inline bool operator !=(_In_ const AABB &b) const
		{
			return Position != b.Position || Size != b.Size;
		}

		/* Scales the box by a specified amount. */
		_Check_return_ inline AABB operator *(_In_ float scalar) const
		{
			AABB result(Position + Size * 0.5f, Vector3());
			result.Inflate(Size.X * scalar, Size.Y * scalar, Size.Z * scalar);
			return result;
		}

		/* Transforms the box with the specified matrix. */
		_Check_return_ AABB operator *(_In_ const Matrix &m) const;
		/* Gets the corner at the specified index. */
		_Check_return_ Vector3 operator [](_In_ size_t idx) const;

		/* Gets whether the box has a size of zero. */
		_Check_return_ inline bool IsEmpty(void) const
		{
			return Size == Vector3();
		}

		/* Gets whether the box has a size of zero and is at the position origin. */
		_Check_return_ inline bool IsUseless(void) const
		{
			return Position == Vector3() && IsEmpty();
		}

		/* Gets the absolute width of the box. */
		_Check_return_ inline float GetWidth(void) const
		{
			return fabsf(Size.X);
		}

		/* Gets the absolute height of the box. */
		_Check_return_ inline float GetHeight(void) const
		{
			return fabsf(Size.Y);
		}

		/* Gets the absolute depth of the box. */
		_Check_return_ inline float GetDepth(void) const
		{
			return fabsf(Size.Z);
		}

		/* Gets the rightmost face of the box. */
		_Check_return_ inline float GetRight(void) const
		{
			return Size.X > 0.0f ? (Position.X + Size.X) : Position.X;
		}

		/* Gets the leftmost face of the box. */
		_Check_return_ inline float GetLeft(void) const
		{
			return Size.X > 0.0f ? Position.X : (Position.X + Size.X);
		}

		/* Gets the topmost face of the box. */
		_Check_return_ inline float GetTop(void) const
		{
			return Size.Y > 0.0f ? (Position.Y + Size.Y) : Position.Y;
		}

		/* Gets the bottommost face of the box. */
		_Check_return_ inline float GetBottom(void) const
		{
			return Size.Y > 0.0f ? Position.Y : (Position.Y + Size.Y);
		}

		/* Gets the frontmost face of the box. */
		_Check_return_ inline float GetFront(void) const
		{
			return Size.Z > 0.0f ? Position.Z : (Position.Z + Size.Z);
		}

		/* Gets the backmost face of the box. */
		_Check_return_ inline float GetBack(void) const
		{
			return Size.Z > 0.0f ? (Position.Z + Size.Z) : Position.Z;
		}

		/* Gets the center point of the box. */
		_Check_return_ inline Vector3 GetCenter(void) const
		{
			return Position + Size * 0.5f;
		}

		/* Mixes the two boxes with a specified amount. */
		_Check_return_ static AABB Mix(_In_ const AABB &first, _In_ const AABB &second, _In_ float a);

		/* Expands the box from all faces by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical, _In_ float depth);
		/* Squares out the box making the horizontal, verticel and depth equal to the highest of the three. */
		void Square(void);
		/* Creates a box that contains the two input boxes. */
		_Check_return_ AABB Merge(_In_ const AABB &second) const;
		/* Creates a box that contains the input box and the specified point. */
		_Check_return_ AABB Merge(_In_ Vector3 point) const;
		/* Clamps a specified point within the bounds of the box. */
		_Check_return_ Vector3 Clamp(_In_ Vector3 point) const;
		/* Checks whether a point is within the box. */
		_Check_return_ bool Contains(_In_ Vector3 point) const;
		/* Checks whether a box is fully within the box. */
		_Check_return_ bool Contains(_In_ const AABB &b) const;
		/* Checks whether a box overlaps with the box. */
		_Check_return_ bool Overlaps(_In_ const AABB &b) const;
		/* Gets the overlap of a box over the box. */
		_Check_return_ AABB GetOverlap(_In_ const AABB &b) const;
	};

	/* Transforms the box with the specified matrix. */
	_Check_return_ inline AABB operator *(_In_ const Matrix &m, _In_ const AABB &b)
	{
		return b * m;
	}
}