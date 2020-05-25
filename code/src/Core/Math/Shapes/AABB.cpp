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

void Pu::AABB::Inflate(float horizontal, float vertical, float depth)
{
	const Vector3 adder(horizontal * 0.5f, vertical * 0.5f, depth * 0.5f);
	LowerBound -= adder;
	UpperBound += adder;
}

void Pu::AABB::MergeInto(Vector3 p)
{
	LowerBound = min(LowerBound, p);
	UpperBound = max(UpperBound, p);
}

void Pu::AABB::MergeInto(const AABB & second)
{
	LowerBound = min(LowerBound, second.LowerBound);
	UpperBound = max(UpperBound, second.UpperBound);
}