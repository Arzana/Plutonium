#include "Graphics/Color.h"
#include "Core/Math/Interpolation.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Collections/Vector.h"

using namespace Pu;

constexpr float CONV_MOD = recip(255.0f);

Color Pu::Color::FromNonPremultiplied(int32 r, int32 g, int32 b, int32 a)
{
	/* On debug check if input is valid. */
#if defined(_DEBUG)
	if (r < 0 || r > maxv<byte>()) Log::Warning("Red value(%d) is outside color boundaries(0 >= R <= 255)!", r);
	if (g < 0 || g > maxv<byte>()) Log::Warning("Green value(%d) is outside color boundaries(0 >= G <= 255)!", g);
	if (b < 0 || b > maxv<byte>()) Log::Warning("Blue value(%d) is outside color boundaries(0 >= B <= 255)!", b);
	if (a < 0 || a > maxv<byte>()) Log::Warning("Alpha value(%d) is outside color boundaries(0 >= A <= 255)!", a);
#endif

	/* Multiply alpha over color. */
	const float v = a * CONV_MOD;
	return Color(static_cast<byte>(ipart(r * v)), static_cast<byte>(ipart(g * v)), static_cast<byte>(ipart(b * v)), static_cast<byte>(a));
}

Color Pu::Color::Lerp(Color a, Color b, float v)
{
	const int32 alpha = static_cast<int32>(lerp(static_cast<float>(a.A), static_cast<float>(b.A), v));
	const int32 red = static_cast<int32>(lerp(static_cast<float>(a.R), static_cast<float>(b.R), v));
	const int32 green = static_cast<int32>(lerp(static_cast<float>(a.G), static_cast<float>(b.G), v));
	const int32 blue = static_cast<int32>(lerp(static_cast<float>(a.B), static_cast<float>(b.B), v));
	return FromNonPremultiplied(red, green, blue, alpha);
}

Color Pu::Color::Lerp(Color a, Color b, float c, float d, float v)
{
	return Color::Lerp(a, b, ilerp(c, d, v));
}

Color Pu::Color::Random(byte gain, bool unique)
{
	/* Create static buffer to store all unique colors. */
	static vector<Color> usedColors;

	Color result;
	do
	{
		/* Create random rgb value with minimum gain value. */
		const byte r = static_cast<byte>(random(gain, maxv<byte>()));
		const byte g = static_cast<byte>(random(gain, maxv<byte>()));
		const byte b = static_cast<byte>(random(gain, maxv<byte>()));

		/* Create result and return if not unique. */
		result = Color(r, g, b);
		if (!unique) return result;

		/* Loop untill a unique random color is found. */
	} while (usedColors.contains(result));

	/* Push unique color to the buffer. */
	usedColors.push_back(result);
	return result;
}

Color Pu::Color::operator+(Color other) const
{
	return Color(static_cast<byte>(R + other.R), static_cast<byte>(G + other.G), static_cast<byte>(B + other.B), static_cast<byte>(A + other.A));
}

Color Pu::Color::operator-(Color other) const
{
	return Color(static_cast<byte>(R - other.R), static_cast<byte>(G - other.G), static_cast<byte>(B - other.B), static_cast<byte>(A - other.A));
}

Color Pu::Color::operator*(float scalar) const
{
	return Color(byte(R * scalar), byte(G * scalar), byte(B * scalar), byte(A * scalar));
}

Color Pu::Color::operator*(Color other) const
{
	const Vector4 result = ToVector4() * other.ToVector4();
	return Color(result.X, result.Y, result.Z, result.W);
}

Vector4 Pu::Color::ToVector4(void) const
{
	return Vector4(B * CONV_MOD, G * CONV_MOD, R * CONV_MOD, A * CONV_MOD);
}

byte * Pu::Color::ToArray(void) const
{
	return reinterpret_cast<byte*>(const_cast<Color*>(this));
}

ClearColorValue Pu::Color::ToClearColor(void) const
{
	/* 
	ToVector4 returns with the format BGRA which is what the surface expects 
	but clear color is always RGBA so we must rearange the order.
	*/
	const Vector4 color = ToVector4();
	return { color.Z, color.Y, color.X, color.W };
}