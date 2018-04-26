#include "Graphics\Color.h"
#include "Core\Math\Basics.h"
#include "Core\Math\Interpolation.h"
#include "Core\Diagnostics\Logging.h"

using namespace Plutonium;

constexpr int32 MASK_OPAQUE = 0x000000FF;
constexpr int32 MASK_RED = 0xFF000000;
constexpr int32 MASK_BLUE = 0x0000FF00;
constexpr int32 MASK_GREEN = 0x00FF0000;
constexpr float CONV_MOD = recip(255.0f);

const Color Color::Transparent =	Color();
const Color Color::Black =			Color(MASK_OPAQUE);
const Color Color::Red =			Color(MASK_OPAQUE | MASK_RED);
const Color Color::Green =			Color(MASK_OPAQUE | MASK_GREEN);
const Color Color::Blue =			Color(MASK_OPAQUE | MASK_BLUE);
const Color Color::Cyan =			Color(MASK_OPAQUE | MASK_BLUE | MASK_GREEN);
const Color Color::Magenta =		Color(MASK_OPAQUE | MASK_RED | MASK_BLUE);
const Color Color::Yellow =			Color(MASK_OPAQUE | MASK_RED | MASK_GREEN);
const Color Color::White =			Color(MASK_OPAQUE | MASK_RED | MASK_BLUE | MASK_GREEN);
const Color Color::SunDay =			Color(0xFFFFDCFF);
const Color Color::SunDawn =		Color(0xF7B668FF);

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

Vector4 Plutonium::Color::ToVector4(void) const
{
	return Vector4(R * CONV_MOD, G * CONV_MOD, B * CONV_MOD, A * CONV_MOD);
}
