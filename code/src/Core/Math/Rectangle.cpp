#include "Core\Math\Rectangle.h"

void Pu::Rectangle::Inflate(float horizontal, float vertical)
{
	const Vector2 adder(horizontal * 0.5f, vertical * 0.5f);
	LowerBound -= adder;
	UpperBound += adder;
}

Pu::Rectangle Pu::Rectangle::Merge(const Rectangle & second) const
{
	return Rectangle(min(LowerBound, second.LowerBound), max(UpperBound, second.UpperBound));
}

Pu::Rectangle Pu::Rectangle::Merge(Vector2 point) const
{
	return Rectangle(min(LowerBound, point), max(UpperBound, point));
}

Pu::Vector2 Pu::Rectangle::Separate(const Rectangle & second) const
{
	return Merge(second).GetSize() - GetOverlap(second).GetSize();
}

bool Pu::Rectangle::Contains(Vector2 point) const
{
	return LowerBound.X <= point.X && UpperBound.X >= point.X
		&& LowerBound.Y <= point.Y && UpperBound.Y >= point.Y;
}

bool Pu::Rectangle::Contains(const Rectangle & r) const
{
	return LowerBound.X <= r.LowerBound.X && UpperBound.X >= r.UpperBound.X
		&& LowerBound.Y <= r.LowerBound.Y && UpperBound.Y >= r.UpperBound.Y;
}

bool Pu::Rectangle::Overlaps(const Rectangle & r) const
{
	return LowerBound.X <= r.UpperBound.X && UpperBound.X >= r.LowerBound.X
		&& LowerBound.Y <= r.UpperBound.Y && UpperBound.Y >= r.LowerBound.Y;
}

Pu::Rectangle Pu::Rectangle::GetOverlap(const Rectangle & r) const
{
	const Vector2 low = max(LowerBound, r.LowerBound);
	const Vector2 upp = min(UpperBound, r.UpperBound);
	return upp.X < low.X || upp.Y < low.Y ? Rectangle() : Rectangle(low, upp);
}