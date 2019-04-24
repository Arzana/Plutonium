#pragma once
#include "Vector2.h"

namespace Pu
{
	/* Defines a 2D rectangle. */
	struct Rectangle
	{
		/* The oringin of the rectangle. */
		Vector2 Position;
		/* The size of the rectangle. */
		Vector2 Size;

		/* Initializes a new instance of a empty rectangle. */
		Rectangle(void)
			: Position(), Size()
		{}

		/* Initialize a new instance of a rectangle with a specified size. */
		Rectangle(_In_ Vector2 size)
			: Position(), Size(size)
		{}

		/* Initializes a new instance of a rectangle at a specified position and with a specified size. */
		Rectangle(_In_ Vector2 pos, _In_ Vector2 size)
			: Position(pos), Size(size)
		{}

		/* Initializes a new instance of a rectangle at a specified position and with a specified size. */
		Rectangle(_In_ float x, _In_ float y, _In_ float w, _In_ float h)
			: Position(x, y), Size(w, h)
		{}

		/* Checks whether the two rectangles are equal. */
		_Check_return_ inline bool operator ==(_In_ const Rectangle &r) const
		{
			return Position == r.Position && Size == r.Size;
		}

		/* Checks whether the two rectangles differ. */
		_Check_return_ inline bool operator !=(_In_ const Rectangle &r) const
		{
			return Position != r.Position || Size != r.Size;
		}

		/* Implicitly converts the rectangle to a string. */
		_Check_return_ inline operator string() const
		{
			string result("[x: ");
			result += string::from(Position.X);
			result += ", Y: ";
			result += string::from(Position.Y);
			result += ", W: ";
			result += string::from(Size.X);
			result += ", H: ";
			result += string::from(Size.Y);
			return result += ']';
		}

		/* Gets whether the rectangle has a size of zero. */
		_Check_return_ inline bool IsEmpty(void) const
		{
			return Size == Vector2();
		}

		/* Gets the absolute width of the rectangle. */
		_Check_return_ inline float GetWidth(void) const
		{
			return fabs(Size.X);
		}

		/* Gets the absolute height of the rectangle. */
		_Check_return_ inline float GetHeight(void) const
		{
			return fabsf(Size.Y);
		}

		/* Gets the rightmost edge of the rectangle. */
		_Check_return_ inline float GetRight(void) const
		{
			return Size.X > 0.0f ? (Position.X + Size.X) : Position.X;
		}

		/* Gets the leftmost edge of the rectangle. */
		_Check_return_ inline float GetLeft(void) const
		{
			return Size.X > 0.0f ? Position.X : (Position.X + Size.X);
		}

		/* Gets the topmost edge of the rectangle. */
		_Check_return_ inline float GetTop(void) const
		{
			return Size.Y > 0.0f ? Position.Y : (Position.Y + Size.Y);
		}

		/* Gets the bottommost edge of the rectangle. */
		_Check_return_ inline float GetBottom(void) const
		{
			return Size.Y > 0.0f ? (Position.Y + Size.Y) : Position.Y;
		}

		/* Gets the center point of the rectangle. */
		_Check_return_ inline Vector2 GetCenter(void) const
		{
			return Position + Size * 0.5f;
		}

		/* Expands the rectangle from all edges by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical);
		/* Creates a rectangle that contains the two input rectangles. */
		_Check_return_ Rectangle Merge(_In_ const Rectangle &second) const;
		/* Gets the non overlapping space of the two input rectangles. */
		_Check_return_ Vector2 Separate(_In_ const Rectangle &second) const;
		/* Checks whether a point is within the rectangle. */
		_Check_return_ bool Contains(_In_ Vector2 point) const;
		/* Checks whether a rectangle is fully within the rectangle. */
		_Check_return_ bool Contains(_In_ const Rectangle &r) const;
		/* Checks whether a rectangle overlaps with the rectangle. */
		_Check_return_ bool Overlaps(_In_ const Rectangle &r) const;
		/* Gets the overlap of a rectangle over the rectangle. */
		_Check_return_ Rectangle GetOverlap(_In_ const Rectangle &r) const;
	};
}