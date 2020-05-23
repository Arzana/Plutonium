#include "Core/Math/Shapes/AABB.h"
#include "Core/Math/Interpolation.h"

using namespace Pu;

AABB Pu::AABB::operator*(const Matrix & m) const
{
	return AABB(m * LowerBound, m * UpperBound);
}

Vector3 Pu::AABB::operator[](size_t idx) const
{
	switch (idx)
	{
	case(0):	// Front Bottom Left
		return LowerBound;
	case(1):	// Front Bottom Right
		return Vector3(UpperBound.X, LowerBound.Y, LowerBound.Z);
	case(2):	// Front Top Right
		return Vector3(UpperBound.X, UpperBound.Y, LowerBound.Z);
	case(3):	// Front Top Left
		return Vector3(LowerBound.X, UpperBound.Y, LowerBound.Z);
	case(4):	// Back Bottom Left.
		return Vector3(LowerBound.X, LowerBound.Y, UpperBound.Z);
	case(5):	// Back Bottom Right
		return Vector3(UpperBound.X, LowerBound.Y, UpperBound.Z);
	case(6):	// Back Top Right
		return UpperBound;
	case(7):	// Back Top Left
		return Vector3(LowerBound.X, UpperBound.Y, UpperBound.Z);
	default:
		return Vector3();
	}
}

AABB Pu::AABB::operator+(Vector3 offset) const
{
	return AABB(LowerBound + offset, UpperBound + offset);
}

AABB Pu::AABB::Mix(const AABB & first, const AABB & second, float a)
{
	return AABB(lerp(first.LowerBound, second.LowerBound, a), lerp(first.UpperBound, second.UpperBound, a));
}

void Pu::AABB::Inflate(float horizontal, float vertical, float depth)
{
	const Vector3 adder(horizontal * 0.5f, vertical * 0.5f, depth * 0.5f);
	LowerBound -= adder;
	UpperBound += adder;
}

AABB Pu::AABB::Merge(const AABB & second) const
{
	return AABB(min(LowerBound, second.LowerBound), max(UpperBound, second.UpperBound));
}

AABB Pu::AABB::Merge(Vector3 point) const
{
	return AABB(min(LowerBound, point), max(UpperBound, point));
}

void Pu::AABB::MergeInto(const AABB & second)
{
	LowerBound = min(LowerBound, second.LowerBound);
	UpperBound = max(UpperBound, second.UpperBound);
}

bool Pu::AABB::Contains(const AABB & r) const
{
	return LowerBound.X <= r.LowerBound.X && UpperBound.X >= r.UpperBound.X
		&& LowerBound.Y <= r.LowerBound.Y && UpperBound.Y >= r.UpperBound.Y
		&& UpperBound.Z <= r.LowerBound.Z && UpperBound.Z >= r.UpperBound.Z;
}

AABB Pu::AABB::GetOverlap(const AABB & r) const
{
	const Vector3 low = max(LowerBound, r.LowerBound);
	const Vector3 upp = min(UpperBound, r.UpperBound);
	return upp.X < low.X || upp.Y < low.Y || upp.Z < low.Z ? AABB() : AABB(low, upp);
}

float Pu::AABB::GetDistance(Vector3 point) const
{
	return max(LowerBound - point, point - UpperBound).Length();
}