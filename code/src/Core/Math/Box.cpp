#include "Core\Math\Box.h"
#include "Core\Math\Basics.h"

using namespace Plutonium;

Box Plutonium::Box::Merge(const Box & first, const Box & second)
{
	const float r = max(first.GetRight(), second.GetRight());
	const float l = min(first.GetLeft(), second.GetLeft());
	const float t = min(first.GetTop(), second.GetTop());
	const float d = max(first.GetBottom(), second.GetBottom());
	const float f = min(first.GetFront(), second.GetFront());
	const float b = max(first.GetBack(), second.GetBack());
	return Box(l, t, f, r - l, d - t, b - f);
}

void Plutonium::Box::Inflate(float horizontal, float vertical, float depth)
{
	horizontal *= 0.5f;
	vertical *= 0.5f;
	depth *= 0.5f;

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

	if (Size.Z > 0.0f)
	{
		Position.Z -= depth;
		Size.Z += depth;
	}
	else
	{
		Position.Z += depth;
		Size.Z -= depth;
	}
}

bool Plutonium::Box::Contains(Vector3 point) const
{
	return GetLeft() <= point.X && GetRight() >= point.X
		&& GetTop() <= point.Y && GetBottom() >= point.Y
		&& GetFront() <= point.Z && GetBack() >= point.Z;
}

bool Plutonium::Box::Contains(const Box & b) const
{
	return GetLeft() <= b.GetLeft() && GetRight() >= b.GetRight()
		&& GetTop() <= b.GetTop() && GetBottom() >= b.GetBottom()
		&& GetFront() <= b.GetFront() && GetBack() >= b.GetBack();
}

bool Plutonium::Box::Overlaps(const Box & b) const
{
	return GetLeft() <= b.GetRight() && GetRight() >= b.GetLeft()
		&& GetTop() <= b.GetBottom() && GetBottom() >= b.GetTop()
		&& GetFront() <= b.GetBack() && GetBack() >= b.GetFront();
}

Box Plutonium::Box::GetOverlap(const Box & b) const
{
	const float xl = max(GetLeft(), b.GetLeft());
	const float xs = min(GetRight(), b.GetRight());
	if (xs < xl) return Box();

	const float yl = max(GetTop(), b.GetTop());
	const float ys = min(GetBottom(), b.GetBottom());
	if (ys < yl) return Box();

	const float zl = max(GetFront(), b.GetFront());
	const float zs = min(GetBack(), b.GetBack());
	if (zs < zl) return Box();

	return Box(xl, yl, zl, xs - xl, ys - yl, zs - zl);
}
