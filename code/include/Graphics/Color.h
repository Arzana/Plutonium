#pragma once
#include "Core\Math\Constants.h"
#include "Core\Math\Vector4.h"

namespace Plutonium
{
	/* Defines a 32 bit RGBA color. */
	struct Color
	{
	public:
		/* Defines a black transparent color. */
		const static Color Transparent;
		/* Defines a black opaque color. */
		const static Color Black;
		/* Defines a red opaque color. */
		const static Color Red;
		/* Defines a green opaque color. */
		const static Color Green;
		/* Defines a blue opaque color. */
		const static Color Blue;
		/* Defines a cyan opaque color. */
		const static Color Cyan;
		/* Defines a magenta opaque color. */
		const static Color Magenta;
		/* Defines a yellow opaque color. */
		const static Color Yellow;
		/* Defines a white opaque color. */
		const static Color White;

		union
		{
			struct
			{
				/* The red amount of this color. */
				byte R;
				/* The green amount of this color. */
				byte G;
				/* The blue amount of this color. */
				byte B;
				/* The alpha (transparency) of this color. */
				byte A;
			};

			/* The color as a 32 bit unsigned int. */
			uint32 Packed;
		};

		/* Initializes a new instance of a color as transparent. */
		Color(void)
			: Packed(0)
		{}

		/* Initializes a new instance of a color from a specified packed value. */
		Color(uint32 packed)
			: Packed(packed)
		{}

		/* Initializes a new instance of a opaque color. */
		Color(byte r, byte g, byte b)
			: R(r), G(g), B(b), A(255)
		{}

		/* Initializes a new instance of a specific color. */
		Color(byte r, byte g, byte b, byte a)
			: R(r), G(g), B(b), A(a)
		{}

		/* Creates a color from a non premultiplied alpha value. */
		_Check_return_ static Color FromNonPremultiplied(_In_ int32 r, _In_ int32 g, _In_ int32 b, _In_ int32 a);
		/* Lerps between two specified colors with a specified amount. */
		_Check_return_ static Color Lerp(_In_ Color a, _In_ Color b, _In_ float v);

		/* Adds two colors together. */
		_Check_return_ Color operator +(_In_ Color other) const;
		/* Subtracts a specifed color from this color. */
		_Check_return_ Color operator -(_In_ Color other) const;
		/* Multiples the color by a specified scalar. */
		_Check_return_ Color operator *(_In_ float scalar) const;

		/* Checks whether two colors are equal. */
		_Check_return_ inline bool operator ==(_In_ Color other) const
		{
			return Packed == other.Packed;
		}

		/* Checks whether two colors differ. */
		_Check_return_ inline bool operator !=(_In_ Color other) const
		{
			return Packed != other.Packed;
		}

		/* Gets the color as a four dimensional vector. */
		_Check_return_ Vector4 ToVector4(void) const;
	};
}