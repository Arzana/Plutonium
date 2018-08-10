#include "Core\Math\Box.h"
#include "Core\Math\VInterpolation.h"

using namespace Plutonium;

Box Plutonium::Box::Mix(const Box & first, const Box & second, float a)
{
	return Box(lerp(first.Position, second.Position, a), lerp(first.Size, second.Size, a));
}

Box Plutonium::Box::Merge(const Box & first, const Box & second)
{
	if (first.IsEmpty()) return second;
	if (second.IsEmpty()) return first;

	const float r = max(first.GetRight(), second.GetRight());
	const float l = min(first.GetLeft(), second.GetLeft());
	const float t = max(first.GetTop(), second.GetTop());
	const float d = min(first.GetBottom(), second.GetBottom());
	const float b = max(first.GetBack(), second.GetBack());
	const float f = min(first.GetFront(), second.GetFront());
	return Box(l, d, f, r - l, t - d, b - f);
}

void Plutonium::Box::Inflate(float horizontal, float vertical, float depth)
{
	horizontal *= 0.5f;
	vertical *= 0.5f;
	depth *= 0.5f;

	if (Size.X >= 0.0f)
	{
		Position.X -= horizontal;
		Size.X += horizontal;
	}
	else
	{
		Position.X += horizontal;
		Size.X -= horizontal;
	}

	if (Size.Y >= 0.0f)
	{
		Position.Y -= vertical;
		Size.Y += vertical;
	}
	else
	{
		Position.Y += vertical;
		Size.Y -= vertical;
	}

	if (Size.Z >= 0.0f)
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

void Plutonium::Box::Square(void)
{
	Size = Vector3(max(max(Size.X, Size.Y), Size.Z));
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

bool Plutonium::Box::HitTestRay(Vector3 origin, Vector3 dir, Vector3 * point)
{
	enum Quadrant
	{
		Right,
		Left,
		Middle
	};

	float min[3] = { GetLeft(), GetBottom(), GetFront() };
	float max[3] = { GetRight(), GetTop(), GetBack() };

	bool inside = true;
	Quadrant quadrant[3];
	float candidatePlane[3];

	/* Find the candidate planes. */
	for (size_t i = 0; i < 3; i++)
	{
		if (origin.f[i] < min[i])
		{
			quadrant[i] = Left;
			candidatePlane[i] = min[i];
			inside = false;
		}
		else if (origin.f[i] > max[i])
		{
			quadrant[i] = Right;
			candidatePlane[i] = max[i];
			inside = false;
		}
		else quadrant[i] = Middle;
	}

	/* Ray origionates from withing the box. */
	if (inside)
	{
		if (point) *point = origin;
		return true;
	}

	/* Calculate the T distance to candidate planes. */
	float maxT[3];
	for (size_t i = 0; i < 3; i++) maxT[i] = (quadrant[i] != Middle && dir.f[i] != 0.0f) ? ((candidatePlane[i] - origin.f[i]) / dir.f[i]) : -1.0f;

	/* Get the index of the largest plane distance. */
	size_t lplane = 0;
	for (size_t i = 0; i < 3; i++)
	{
		if (maxT[lplane] < maxT[i]) lplane = i;
	}

	/* Check if the final candidate is actually inside the box. */
	if (maxT[lplane] < 0.0f) return false;

	float coord[3];
	for (size_t i = 0; i < 3; i++)
	{
		if (lplane != i)
		{
			coord[i] = origin.f[i] + maxT[lplane] * dir.f[i];
			if (coord[i] < min[i] || coord[i] > max[i]) return false;
		}
		else coord[i] = candidatePlane[i];
	}

	/* Ray has hit the box. */
	if (point) *point = Vector3(coord[0], coord[1], coord[2]);
	return true;
}