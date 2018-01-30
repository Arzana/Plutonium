#include "Core\Math\Rectangle.h"
#include "Core\Math\Basics.h"

using namespace Plutonium;

Rectangle Plutonium::Rectangle::Merge(const Rectangle & first, const Rectangle & second)
{
	const float r = max(first.GetRight(), second.GetRight());
	const float l = min(first.GetLeft(), second.GetLeft());
	const float t = min(first.GetTop(), second.GetTop());
	const float b = max(first.GetBottom(), second.GetBottom());
	return Rectangle(l, t, r - l, b - t);
}

void Plutonium::Rectangle::Inflate(float horizontal, float vertical)
{
	horizontal *= 0.5f;
	vertical *= 0.5f;

	if (Size.X > 0.0f)
	{
		Position.X -= horizontal;
		Size.X += horizontal;
	}
	else
	{
		Position.X += horizontal;
		Size.X -= horizontal;
	}

	if (Size.Y > 0.0f)
	{
		Position.Y -= vertical;
		Size.Y += vertical;
	}
	else
	{
		Position.Y += vertical;
		Size.Y -= vertical;
	}
}

bool Plutonium::Rectangle::Contains(Vector2 point) const
{
	return GetLeft() < point.X && GetRight() > point.X && GetTop() < point.Y && GetBottom() > point.Y;
}

bool Plutonium::Rectangle::Contains(const Rectangle & r) const
{
	return GetLeft() < r.GetLeft() && GetRight() > r.GetRight() && GetTop() < r.GetTop() && GetBottom() > r.GetBottom();
}

bool Plutonium::Rectangle::Overlaps(const Rectangle & r) const
{
	return GetLeft() <= r.GetRight() && GetRight() >= r.GetLeft() && GetTop() <= r.GetBottom() && GetBottom() >= r.GetTop();
}

Rectangle Plutonium::Rectangle::GetOverlap(const Rectangle & r) const
{
	const float xl = max(GetLeft(), r.GetLeft());
	const float xs = min(GetRight(), r.GetRight());
	if (xs < xl) return Rectangle();

	const float yl = max(GetTop(), r.GetTop());
	const float ys = min(GetBottom(), r.GetBottom());
	if (ys < yl) return Rectangle();

	return Rectangle(xl, yl, xs - xl, ys - yl);
}
