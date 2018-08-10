#pragma once
#include "Matrix.h"

namespace Plutonium
{
	/* Defines a 3D box. */
	struct Box
	{
		/* The origin of the box. */
		Vector3 Position;
		/* The size of the box. */
		Vector3 Size;

		/* Initializes a new instance of an empty box. */
		Box(void)
			: Position(), Size()
		{}

		/* Initializes a new instance of a box with a specified size. */
		Box(_In_ Vector3 size)
			: Position(), Size(size)
		{}

		/* Initializes a new instance of a box with a specified orinin and size. */
		Box(_In_ Vector3 pos, _In_ Vector3 size)
			: Position(pos), Size(size)
		{}

		/* Initializes a new instance of a box with a specified origin and size. */
		Box(_In_ float x, _In_ float y, _In_ float z, _In_ float w, _In_ float h, _In_ float d)
			: Position(x, y, z), Size(w, h, d)
		{}

		/* Checks whether the two boxes are equal. */
		_Check_return_ inline bool operator ==(_In_ const Box &b) const
		{
			return Position == b.Position && Size == b.Size;
		}

		/* Checks whether the two boxes differ. */
		_Check_return_ inline bool operator !=(_In_ const Box &b) const
		{
			return Position != b.Position || Size != b.Size;
		}

		/* Scales the box by a specified amount. */
		_Check_return_ inline Box operator *(_In_ float scalar) const
		{
			Box result(Position + Size * 0.5f, Vector3::Zero());
			result.Inflate(Size.X * scalar, Size.Y * scalar, Size.Z * scalar);
			return result;
		}

		/* Transforms the box with the specified matrix. */
		_Check_return_ inline Box operator *(_In_ const Matrix &m) const
		{
			Vector3 p1 = m * Position;
			Vector3 p2 = m * (Position + Size);
			return Box(p1, p2 - p1);
		}

		/* Gets whether the box has a size of zero. */
		_Check_return_ inline bool IsEmpty(void) const
		{
			return Size == Vector3::Zero();
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
			return Size.Y > 0.0f ?  Position.Y : (Position.Y + Size.Y);
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
		_Check_return_ static Box Mix(_In_ const Box &first, _In_ const Box &second, _In_ float a);
		/* Creates a box that contains the two input boxes. */
		_Check_return_ static Box Merge(_In_ const Box &first, _In_ const Box &second);
		/* Expands the box from all faces by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical, _In_ float depth);
		/* Squares out the box making the horizontal, verticel and depth equal to the highest of the three. */
		void Square(void);
		/* Checks whether a point is within the box. */
		_Check_return_ bool Contains(_In_ Vector3 point) const;
		/* Checks whether a box is fully within the box. */
		_Check_return_ bool Contains(_In_ const Box &b) const;
		/* Checks whether a box overlaps with the box. */
		_Check_return_ bool Overlaps(_In_ const Box &b) const;
		/* Gets the overlap of a box over the box. */
		_Check_return_ Box GetOverlap(_In_ const Box &b) const;
		/* Checks whether the specified ray has hit the bounding box, optionally outputs the point of the hit. */
		_Check_return_ bool HitTestRay(_In_ Vector3 origin, _In_ Vector3 dir, _Out_opt_ Vector3 *point);
	};

	/* Transforms the box with the specified matrix. */
	_Check_return_ inline Box operator *(_In_ const Matrix &m, _In_ const Box &b)
	{
		return b * m;
	}
}