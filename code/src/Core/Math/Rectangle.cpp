#include "Core\Math\Rectangle.h"

void Pu::Rectangle::Inflate(float horizontal, float vertical)
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

Pu::Rectangle Pu::Rectangle::Merge(const Rectangle & second) const
{
	if (IsEmpty()) return second;
	if (second.IsEmpty()) return *this;

	const float r = max(GetRight(), second.GetRight());
	const float l = min(GetLeft(), second.GetLeft());
	const float t = min(GetTop(), second.GetTop());
	const float b = max(GetBottom(), second.GetBottom());
	return Rectangle(l, t, r - l, b - t);
}

Pu::Vector2 Pu::Rectangle::Separate(const Rectangle & second) const
{
	const Rectangle merge = Merge(second);
	const Rectangle overlap = GetOverlap(second);
	return merge.Size - overlap.Size;
}

bool Pu::Rectangle::Contains(Vector2 point) const
{
	return GetLeft() <= point.X && GetRight() >= point.X && GetTop() <= point.Y && GetBottom() >= point.Y;
}

bool Pu::Rectangle::Contains(const Rectangle & r) const
{
	return GetLeft() <= r.GetLeft() && GetRight() >= r.GetRight() && GetTop() <= r.GetTop() && GetBottom() >= r.GetBottom();
}

bool Pu::Rectangle::Overlaps(const Rectangle & r) const
{
	return GetLeft() <= r.GetRight() && GetRight() >= r.GetLeft() && GetTop() <= r.GetBottom() && GetBottom() >= r.GetTop();
}

Pu::Rectangle Pu::Rectangle::GetOverlap(const Rectangle & r) const
{
	const float xl = max(GetLeft(), r.GetLeft());
	const float xs = min(GetRight(), r.GetRight());
	if (xs < xl) return Rectangle();

	const float yl = max(GetTop(), r.GetTop());
	const float ys = min(GetBottom(), r.GetBottom());
	if (ys < yl) return Rectangle();

	return Rectangle(xl, yl, xs - xl, ys - yl);
}
