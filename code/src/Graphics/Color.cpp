#include "Graphics\Color.h"
#include "Core\Math\Basics.h"
#include "Core\Math\Interpolation.h"
#include "Core\Diagnostics\Logging.h"
#include <vector>

using namespace Plutonium;

constexpr float CONV_MOD =			recip(255.0f);

const Color Color::Transparent =		Color();
const Color Color::TransparentWhite =	Color((byte)0xFF, 0xFF, 0xFF, 0x00);
const Color Color::Black =				Color((byte)0x00, 0x00, 0x00);
const Color Color::Red =				Color((byte)0xFF, 0x00, 0x00);
const Color Color::Green =				Color((byte)0x00, 0xFF, 0x00);
const Color Color::Blue =				Color((byte)0x00, 0x00, 0xFF);
const Color Color::Cyan =				Color((byte)0x00, 0xFF, 0xFF);
const Color Color::Magenta =			Color((byte)0xFF, 0x00, 0xFF);
const Color Color::Yellow =				Color((byte)0xFF, 0xFF, 0x00);
const Color Color::White =				Color((byte)0xFF, 0xFF, 0xFF);
const Color Color::SunDay =				Color((byte)0xFF, 0xFF, 0xDC);
const Color Color::SunDawn =			Color((byte)0xF7, 0xB6, 0x68);
const Color Color::Malibu =				Color((byte)0x80, 0x80, 0xFF);
const Color Color::WhiteSmoke =			Color((byte)0xF5, 0xF5, 0xFF);
const Color Color::Lime =				Color((byte)0x7F, 0xFF, 0x00);
const Color Color::Abbey =				Color((byte)0x4C, 0x4F, 0x56);

Color Plutonium::Color::FromNonPremultiplied(int32 r, int32 g, int32 b, int32 a)
{
	/* On debug check if input is valid. */
#if defined(DEBUG)
	LOG_WAR_IF(r < 0 || r > 255, "Red value(%d) is outside color boundaries(0 >= R <= 255)!", r);
	LOG_WAR_IF(g < 0 || g > 255, "Green value(%d) is outside color boundaries(0 >= G <= 255)!", g);
	LOG_WAR_IF(b < 0 || b > 255, "Blue value(%d) is outside color boundaries(0 >= B <= 255)!", b);
	LOG_WAR_IF(a < 0 || a > 255, "Alpha value(%d) is outside color boundaries(0 >= A <= 255)!", a);
#endif

	/* Multiply alpha over color. */
	float v = a * CONV_MOD;
	return Color(static_cast<byte>(ipart(r * v)), static_cast<byte>(ipart(g * v)), static_cast<byte>(ipart(b * v)), a);
}

Color Plutonium::Color::Lerp(Color a, Color b, float v)
{
	int32 alpha = static_cast<int32>(lerp(static_cast<float>(a.A), static_cast<float>(b.A), v));
	int32 red = static_cast<int32>(lerp(static_cast<float>(a.R), static_cast<float>(b.R), v));
	int32 green = static_cast<int32>(lerp(static_cast<float>(a.G), static_cast<float>(b.G), v));
	int32 blue = static_cast<int32>(lerp(static_cast<float>(a.B), static_cast<float>(b.B), v));
	return FromNonPremultiplied(red, green, blue, alpha);
}

Color Plutonium::Color::Lerp(Color a, Color b, float c, float d, float v)
{
	return Color::Lerp(a, b, ilerp(c, d, v));
}

Color Plutonium::Color::Random(byte gain, bool unique)
{
	/* Create static buffer to store all unique colors. */
	static std::vector<Color> usedColors;

	Color result;
	do
	{
		/* Create random rgb value with minimum gain value. */
		byte r = static_cast<byte>(random(gain, maxv<byte>()));
		byte g = static_cast<byte>(random(gain, maxv<byte>()));
		byte b = static_cast<byte>(random(gain, maxv<byte>()));

		/* Create result and return if not unique. */
		result = Color(r, g, b);
		if (!unique) return result;

		/* Loop untill a unique random color is found. */
	} while (std::find(usedColors.begin(), usedColors.end(), result) != usedColors.end());

	/* Push unique color to the buffer. */
	usedColors.push_back(result);
	return result;
}

Color Plutonium::Color::operator+(Color other) const
{
	return Color(static_cast<byte>(R + other.R), static_cast<byte>(G + other.G), static_cast<byte>(B + other.B), static_cast<byte>(A + other.A));
}

Color Plutonium::Color::operator-(Color other) const
{
	return Color(static_cast<byte>(R - other.R), static_cast<byte>(G - other.G), static_cast<byte>(B - other.B), static_cast<byte>(A - other.A));
}

Color Plutonium::Color::operator*(float scalar) const
{
	return Color(byte(R * scalar), byte(G * scalar), byte(B * scalar), byte(A * scalar));
}

Color Plutonium::Color::operator*(Color other) const
{
	Vector4 result = ToVector4() * other.ToVector4();
	return Color(result.X, result.Y, result.Z, result.W);
}

Vector4 Plutonium::Color::ToVector4(void) const
{
	return Vector4(R * CONV_MOD, G * CONV_MOD, B * CONV_MOD, A * CONV_MOD);
}

byte * Plutonium::Color::ToArray(void) const
{
	return reinterpret_cast<byte*>(const_cast<Color*>(this));
}
