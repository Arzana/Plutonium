#pragma once
#include "Vector2.h"

namespace Pu
{
	/* Defines a 2D rectangle. */
	struct Rectangle
	{
		/* The lower bound of the rectangle (position). */
		Vector2 LowerBound;
		/* The upper bound of the rectangle (position + size). */
		Vector2 UpperBound;

		/* Initializes an empty instance of a rectangle. */
		Rectangle(void)
		{}

		/* Initializes a new instance of a rectangle of a specific size. */
		Rectangle(_In_ Vector2 size)
			: UpperBound(size)
		{}

		/* Initializes a new instance of a rectangle. */
		Rectangle(_In_ Vector2 lowerBound, _In_ Vector2 upperBound)
			: LowerBound(lowerBound), UpperBound(upperBound)
		{}

		/* Initializes a new instance of a rectangle. */
		Rectangle(_In_ float x, _In_ float y, _In_ float w, _In_ float h)
			: LowerBound(x, y), UpperBound(x + w, y + h)
		{}

		/* Checks whether the two rectangles are equal. */
		_Check_return_ inline bool operator ==(_In_ const Rectangle &r) const
		{
			return LowerBound == r.LowerBound && UpperBound == r.UpperBound;
		}

		/* Checks whether the two rectangles differ. */
		_Check_return_ inline bool operator !=(_In_ const Rectangle &r) const
		{
			return LowerBound != r.LowerBound || UpperBound != r.UpperBound;
		}

		/* Moves the rectangle with the specified amount. */
		_Check_return_ inline Rectangle operator +(_In_ const Vector2 &r) const
		{
			return Rectangle(LowerBound + r, UpperBound + r);
		}

		/* Moves this rectangle with the specifed amount. */
		_Check_return_ inline Rectangle& operator +=(_In_ const Vector2 &r)
		{
			LowerBound += r;
			UpperBound += r;
			return *this;
		}

		/* Implicitly converts the rectangle to a string. */
		_Check_return_ inline operator string() const
		{
			string result("[x: ");
			result += string::from(LowerBound.X);
			result += ", Y: ";
			result += string::from(LowerBound.Y);
			result += ", W: ";
			result += string::from(GetWidth());
			result += ", H: ";
			result += string::from(GetHeight());
			return result += ']';
		}

		/* Gets whether the rectangle has a size of zero. */
		_Check_return_ inline bool IsEmpty(void) const
		{
			return LowerBound == UpperBound;
		}

		/* Gets the absolute width of the rectangle. */
		_Check_return_ inline float GetWidth(void) const
		{
			return fabs(GetSize().X);
		}

		/* Gets the absolute height of the rectangle. */
		_Check_return_ inline float GetHeight(void) const
		{
			return fabsf(GetSize().Y);
		}

		/* Gets the center point of the rectangle. */
		_Check_return_ inline Vector2 GetCenter(void) const
		{
			return (LowerBound + UpperBound) * 0.5f;
		}

		/* Gets the dimensions of the rectangle. */
		_Check_return_ inline Vector2 GetSize(void) const
		{
			return UpperBound - LowerBound;
		}

		/* Gets the surface area of the rectangle. */
		_Check_return_ inline float GetArea(void) const
		{
			const Vector2 dim = GetSize();
			return dim.X * dim.Y;
		}

		/* Expands the rectangle from all edges by a specified amount. */
		void Inflate(_In_ float horizontal, _In_ float vertical);
		/* Creates a rectangle that contains the two input rectangles. */
		_Check_return_ Rectangle Merge(_In_ const Rectangle &second) const;
		/* Creates a rectangle that contains the input rectangle and the point. */
		_Check_return_ Rectangle Merge(_In_ Vector2 point) const;
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