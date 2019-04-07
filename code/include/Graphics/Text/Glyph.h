#pragma once
#include "Core/Math/Rectangle.h"

namespace Pu
{
	/* Defines all information needed for a glyph within a texture altas. */
	struct Glyph
	{
	public:
		/* The indentifier for this glyph. */
		char32 Key;
		/* The texture bounds of this glyph (range: [0, 1]). */
		Rectangle Bounds;
		/* The actual size of the character. */
		Vector2 Size;
		/* The offset from the render position. */
		Vector2 Bearing;
		/* The directional space added to the position after this character is draw. */
		uint32 Advance;

		/* Initializes an empty instance of a glyph. */
		Glyph(void)
			: Key(U'\0'), Advance(0)
		{}

		/* Copy constructor. */
		Glyph(_In_ const Glyph&) = default;
		/* Move constructor. */
		Glyph(_In_ Glyph&&) = default;

		/* Copy assignment. */
		_Check_return_ Glyph& operator =(_In_ const Glyph&) = default;
		/* Move assignment. */
		_Check_return_ Glyph& operator =(_In_ Glyph&&) = default;

		/* Checks whether tho glyph are equal. */
		_Check_return_ inline bool operator ==(_In_ const Glyph &other) const
		{
			return other.Key == Key;
		}

		/* Checks whether tho glyph differ. */
		_Check_return_ inline bool operator !=(_In_ const Glyph &other) const
		{
			return other.Key != Key;
		}
	};
}