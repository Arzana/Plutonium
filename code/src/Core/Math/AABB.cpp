#include "Core/Math/AABB.h"
#include "Core/Math/Interpolation.h"

using namespace Pu;

void Pu::AABB::Inflate(float horizontal, float vertical, float depth)
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

void Pu::AABB::Square(void)
{
	Size = Vector3(max(max(Size.X, Size.Y), Size.Z));
}

AABB Pu::AABB::operator*(const Matrix & m) const
{
	const Vector3 other = Position + Size;

	const Vector3 x1 = m.GetRight() * Position.X;
	const Vector3 x2 = m.GetRight() * other.X;

	const Vector3 y1 = m.GetUp() * Position.Y;
	const Vector3 y2 = m.GetUp() * other.Y;

	const Vector3 z1 = m.GetBackward() * Position.Z;
	const Vector3 z2 = m.GetBackward() * other.Z;

	const Vector3 p1 = min(x1, x2) + min(y1, y2) + min(z1, z2) + m.GetTranslation();
	const Vector3 p2 = max(x1, x2) + max(y1, y2) + max(z1, z2) + m.GetTranslation();

	return AABB(p1, p2 - p1);
}

Vector3 Pu::AABB::operator[](size_t idx) const
{
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
	default:
		return Vector3();
	}
}

AABB Pu::AABB::Mix(const AABB & first, const AABB & second, float a)
{
	return AABB(lerp(first.Position, second.Position, a), lerp(first.Size, second.Size, a));
}

AABB Pu::AABB::Merge(const AABB & second) const
{
	if (IsUseless()) return second;
	if (second.IsUseless()) return *this;

	const float r = max(GetRight(), second.GetRight());
	const float l = min(GetLeft(), second.GetLeft());
	const float t = max(GetTop(), second.GetTop());
	const float d = min(GetBottom(), second.GetBottom());
	const float b = max(GetBack(), second.GetBack());
	const float f = min(GetFront(), second.GetFront());
	return AABB(l, d, f, r - l, t - d, b - f);
}

AABB Pu::AABB::Merge(Vector3 point) const
{
	if (IsUseless()) return AABB(point, Vector3());

	const float r = max(GetRight(), point.X);
	const float l = min(GetLeft(), point.X);
	const float t = max(GetTop(), point.Y);
	const float d = min(GetBottom(), point.Y);
	const float b = max(GetBack(), point.Z);
	const float f = min(GetFront(), point.Z);
	return AABB(l, d, f, r - l, t - d, b - f);
}

Vector3 Pu::AABB::Clamp(Vector3 point) const
{
	const float x = clamp(point.X, GetLeft(), GetRight());
	const float y = clamp(point.Y, GetBottom(), GetTop());
	const float z = clamp(point.Z, GetFront(), GetBack());
	return Vector3(x, y, z);
}

bool Pu::AABB::Contains(Vector3 point) const
{
	return GetLeft() <= point.X && GetRight() >= point.X
		&& GetTop() > point.Y && GetBottom() < point.Y
		&& GetFront() <= point.Z && GetBack() >= point.Z;
}

bool Pu::AABB::Contains(const AABB & b) const
{
	return GetLeft() <= b.GetLeft() && GetRight() >= b.GetRight()
		&& GetTop() > b.GetTop() && GetBottom() < b.GetBottom()
		&& GetFront() <= b.GetFront() && GetBack() >= b.GetBack();
}

bool Pu::AABB::Overlaps(const AABB & b) const
{
	return GetLeft() <= b.GetRight() && GetRight() >= b.GetLeft()
		&& GetTop() > b.GetBottom() && GetBottom() < b.GetTop()
		&& GetFront() <= b.GetBack() && GetBack() >= b.GetFront();
}

AABB Pu::AABB::GetOverlap(const AABB & b) const
{
	const float xl = max(GetLeft(), b.GetLeft());
	const float xs = min(GetRight(), b.GetRight());
	if (xs < xl) return AABB();

	const float yl = max(GetTop(), b.GetTop());
	const float ys = min(GetBottom(), b.GetBottom());
	if (ys < yl) return AABB();

	const float zl = max(GetFront(), b.GetFront());
	const float zs = min(GetBack(), b.GetBack());
	if (zs < zl) return AABB();

	return AABB(xl, yl, zl, xs - xl, ys - yl, zs - zl);
}