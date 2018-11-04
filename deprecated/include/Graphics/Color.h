#pragma once
#include "Core\Math\Constants.h"
#include "Core\Math\Vector4.h"

namespace Plutonium
{
	/* Defines a 32 bit RGBA color. */
	struct Color
	{
	public:
		union
		{
			/* This project is build to compile with the Microsoft compiler and that does allow this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
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
#pragma warning(pop)

			/* The color as a 32 bit unsigned int. */
			uint32 Packed;
		};

		/* Initializes a new instance of a color as transparent. */
		Color(void)
			: Packed(0)
		{}

		/* Initializes a new instance of a color from a specified packed value. */
		Color(_In_ uint32 packed)
			: Packed(packed)
		{}

		/* Initializes a new instance of a opaque color. */
		Color(_In_ byte r, _In_ byte g, _In_ byte b)
			: R(r), G(g), B(b), A(255)
		{}

		/* Initializes a new instance of a specific color. */
		Color(_In_ byte r, _In_ byte g, _In_ byte b, _In_ byte a)
			: R(r), G(g), B(b), A(a)
		{}

		/* Initializes a new instance of a specified color. */
		Color(_In_ float r, _In_ float g, _In_ float b)
			: R(static_cast<byte>(r * 255.0f)), G(static_cast<byte>(g * 255.0f)), B(static_cast<byte>(b * 255.0f)), A(255)
		{}

		/* Initializes a new instance of a specified color. */
		Color(_In_ float r, _In_ float g, _In_ float b, _In_ float a)
			: R(static_cast<byte>(r * 255.0f)), G(static_cast<byte>(g * 255.0f)), B(static_cast<byte>(b * 255.0f)), A(static_cast<byte>(a * 255.0f))
		{}

		/* Adds two colors together. */
		_Check_return_ Color operator +(_In_ Color other) const;
		/* Subtracts a specifed color from this color. */
		_Check_return_ Color operator -(_In_ Color other) const;
		/* Multiples the color by a specified scalar. */
		_Check_return_ Color operator *(_In_ float scalar) const;
		/* Multiples the color by a specified color. */
		_Check_return_ Color operator *(_In_ Color other) const;

		/* Defines a abbey opaque color. */
		_Check_return_ static inline Color Abbey(void)
		{
			static Color result = Color((byte)0x4C, 0x4F, 0x56);
			return result;
		}

		/* Defines a black opaque color. */
		_Check_return_ static inline Color Black(void)
		{
			static Color result = Color((byte)0x00, 0x00, 0x00);
			return result;
		}

		/* Defines a blue opaque color. */
		_Check_return_ static inline Color Blue(void)
		{
			static Color result = Color((byte)0x00, 0x00, 0xFF);
			return result;
		}

		/* Defines a lime opaque color. */
		_Check_return_ static inline Color Lime(void)
		{
			static Color result = Color((byte)0x7F, 0xFF, 0x00);
			return result;
		}

		/* Defines a cyan opaque color. */
		_Check_return_ static inline Color Cyan(void)
		{
			static Color result = Color((byte)0x00, 0xFF, 0xFF);
			return result;
		}

		/* Defines a green opaque color. */
		_Check_return_ static inline Color Green(void)
		{
			static Color result = Color((byte)0x00, 0xFF, 0x00);
			return result;
		}

		/* Defines the color of a default normal pointing forwards. */
		_Check_return_ static inline Color Malibu(void)
		{
			static Color result = Color((byte)0x80, 0x80, 0xFF);
			return result;
		}

		/* Defines a magenta opaque color. */
		_Check_return_ static inline Color Magenta(void)
		{
			static Color result = Color((byte)0xFF, 0x00, 0xFF);
			return result;
		}

		/* Defines a red opaque color. */
		_Check_return_ static inline Color Red(void)
		{
			static Color result = Color((byte)0xFF, 0x00, 0x00);
			return result;
		}

		/* Defines the color of the sun during the day. */
		_Check_return_ static inline Color SunDay(void)
		{
			static Color result = Color((byte)0xFF, 0xFF, 0xDC);
			return result;
		}

		/* Defines the color of the sun during dawn and dusk. */
		_Check_return_ static inline Color SunDawn(void)
		{
			static Color result = Color((byte)0xF7, 0xB6, 0x68);
			return result;
		}

		/* Defines a black transparent color. */
		_Check_return_ static inline Color Transparent(void)
		{
			static Color result = Color();
			return result;
		}

		/* Defines a white transparent color. */
		_Check_return_ static inline Color TransparentWhite(void)
		{
			static Color result = Color((byte)0xFF, 0xFF, 0xFF, 0x00);
			return result;
		}

		/* Defines a white opaque color. */
		_Check_return_ static inline Color White(void)
		{
			static Color result = Color((byte)0xFF, 0xFF, 0xFF);
			return result;
		}

		/* Defines a white-smoke color.  */
		_Check_return_ static inline Color WhiteSmoke(void)
		{
			static Color result = Color((byte)0xF5, 0xF5, 0xFF);
			return result;
		}

		/* Defines a yellow opaque color. */
		_Check_return_ static inline Color Yellow(void)
		{
			static Color result = Color((byte)0xFF, 0xFF, 0x00);
			return result;
		}

		/* Creates a color from a non premultiplied alpha value. */
		_Check_return_ static Color FromNonPremultiplied(_In_ int32 r, _In_ int32 g, _In_ int32 b, _In_ int32 a);
		/* Lerps between two specified colors with a specified amount. */
		_Check_return_ static Color Lerp(_In_ Color a, _In_ Color b, _In_ float v);
		/* Lerps between two specified colors within a specified range of values. */
		_Check_return_ static Color Lerp(_In_ Color a, _In_ Color b, _In_ float c, _In_ float d, _In_ float v);
		/* Returns a random color. */
		_Check_return_ static Color Random(_In_opt_ byte gain = 0, _In_opt_ bool unique = true);

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
		/* Gets the color as a four length byte array. */
		_Check_return_ byte* ToArray(void) const;
	};
}