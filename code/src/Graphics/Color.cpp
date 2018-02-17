#include "Graphics\Color.h"
#include "Core\Math\Basics.h"
#include "Core\Math\Interpolation.h"
#include "Core\Diagnostics\Logging.h"

using namespace Plutonium;

const Color Color::Transparent =	Color();
const Color Color::Black =			Color(0x000000FF);
const Color Color::Red =			Color(0xFF0000FF);
const Color Color::Green =			Color(0x00FF00FF);
const Color Color::Blue =			Color(0x0000FFFF);
const Color Color::Cyan =			Color(0x00FFFFFF);
const Color Color::Magenta =		Color(0xFF00FFFF);
const Color Color::Yellow =			Color(0xFFFF00FF);
const Color Color::White =			Color(0xFFFFFFFF);

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
	float v = a / 255.0f;
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

Color Plutonium::Color::operator+(Color other) const
{
	return Color(R + other.R, G + other.G, B + other.B, A + other.A);
}

Color Plutonium::Color::operator-(Color other) const
{
	return Color(R - other.R, G - other.G, B - other.B, A - other.A);
}

Color Plutonium::Color::operator*(float scalar) const
{
	return Color(byte(R * scalar), byte(G * scalar), byte(B * scalar), byte(A * scalar));
}

Vector4 Plutonium::Color::ToVector4(void) const
{
	return Vector4(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
}
