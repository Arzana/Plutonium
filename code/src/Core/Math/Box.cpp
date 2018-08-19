#include "Core\Math\Box.h"
#include "Core\Math\VInterpolation.h"
#include "Core/Diagnostics/Logging.h"

using namespace Plutonium;

Box Plutonium::Box::operator*(const Matrix & m) const
{
	const Vector3 other = Position + Size;

	Vector3 x1 = m.GetRight() * Position.X;
	Vector3 x2 = m.GetRight() * other.X;

	Vector3 y1 = m.GetUp() * Position.Y;
	Vector3 y2 = m.GetUp() * other.Y;

	Vector3 z1 = m.GetBackward() * Position.Z;
	Vector3 z2 = m.GetBackward() * other.Z;

	Vector3 p1 = min(x1, x2) + min(y1, y2) + min(z1, z2) + m.GetTranslation();
	Vector3 p2 = max(x1, x2) + max(y1, y2) + max(z1, z2) + m.GetTranslation();

	return Box(p1, p2 - p1);
}

Vector3 Plutonium::Box::operator[](size_t idx) const
{
	ASSERT_IF(idx > 7, "Corner index out of range!");

	switch (idx)
	{
	case(0):	// Front Top Left
		return Position;
	case(1):	// Front Top Right
		return Position + Vector3(Size.X, 0.0f, 0.0f);
	case(2):	// Front Bottom Right
		return Position + Vector3(Size.X, Size.Y, 0.0f);
	case(3):	// Front Bottom Left
		return Position + Vector3(0.0f, Size.Y, 0.0f);
	case(4):	// Back Top Left.
		return Position + Vector3(0.0f, 0.0f, Size.Z);
	case(5):	// Back Top Right
		return Position + Vector3(Size.X, 0.0f, Size.Z);
	case(6):	// Back Bottom Right
		return Position + Size;
	case(7):	// Back Bottom Left
		return Position + Vector3(0.0f, Size.Y, Size.Z);
	}
}

Box Plutonium::Box::Mix(const Box & first, const Box & second, float a)
{
	return Box(lerp(first.Position, second.Position, a), lerp(first.Size, second.Size, a));
}

Box Plutonium::Box::Merge(const Box & first, const Box & second)
{
	if (first.IsUseless()) return second;
	if (second.IsUseless()) return first;

	const float r = max(first.GetRight(), second.GetRight());
	const float l = min(first.GetLeft(), second.GetLeft());
	const float t = max(first.GetTop(), second.GetTop());
	const float d = min(first.GetBottom(), second.GetBottom());
	const float b = max(first.GetBack(), second.GetBack());
	const float f = min(first.GetFront(), second.GetFront());
	return Box(l, d, f, r - l, t - d, b - f);
}

Plutonium::Box Plutonium::Box::Merge(const Box & box, Vector3 point)
{
	if (box.IsUseless()) return Box(point, Vector3::Zero());

	const float r = max(box.GetRight(), point.X);
	const float l = min(box.GetLeft(), point.X);
	const float t = max(box.GetTop(), point.Y);
	const float d = min(box.GetBottom(), point.Y);
	const float b = max(box.GetBack(), point.Z);
	const float f = min(box.GetFront(), point.Z);
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