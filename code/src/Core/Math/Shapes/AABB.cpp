#include "Core/Math/Shapes/AABB.h"
#include "Core/Math/Interpolation.h"
#include "Core/Math/Matrix3_SIMD.h"

using namespace Pu;

AABB Pu::AABB::operator*(Vector3 scalar) const
{
	return AABB(LowerBound * scalar, UpperBound * scalar);
}

AABB Pu::AABB::operator*(const Matrix & m) const
{
	/* Convert the corners to AVX. */
	const ofloat cx = _mm256_set_ps(LowerBound.X, UpperBound.X, UpperBound.X, LowerBound.X, LowerBound.X, LowerBound.X, UpperBound.X, UpperBound.X);
	const ofloat cy = _mm256_set_ps(LowerBound.Y, LowerBound.Y, UpperBound.Y, UpperBound.Y, UpperBound.Y, LowerBound.Y, LowerBound.Y, UpperBound.Y);
	const ofloat cz = _mm256_set_ps(LowerBound.Z, LowerBound.Z, LowerBound.Z, LowerBound.Z, UpperBound.Z, UpperBound.Z, UpperBound.Z, UpperBound.Z);
	
	/* Convert the matrix to AVX. */
	const float *c = m.GetComponents();
	const ofloat m00 = _mm256_set1_ps(c[0]), m10 = _mm256_set1_ps(c[4]), m20 = _mm256_set1_ps(c[8]), m30 = _mm256_set1_ps(c[12]);
	const ofloat m01 = _mm256_set1_ps(c[1]), m11 = _mm256_set1_ps(c[5]), m21 = _mm256_set1_ps(c[9]), m31 = _mm256_set1_ps(c[13]);
	const ofloat m02 = _mm256_set1_ps(c[2]), m12 = _mm256_set1_ps(c[6]), m22 = _mm256_set1_ps(c[10]), m32 = _mm256_set1_ps(c[14]);

	/*
	Transform * Corners 
	We are doing a 3x3 matrix multiplication and adding the translation afterwards.
	This only works because the transformation matrix isn't projecting and we don't care about the w component.
	*/
	ofloat tmpx, tmpy, tmpz;
	_mm256_mat3mul_v3(m00, m01, m02, m10, m11, m12, m20, m21, m22, cx, cy, cz, tmpx, tmpy, tmpz);
	tmpx = _mm256_add_ps(tmpx, m30);
	tmpy = _mm256_add_ps(tmpy, m31);
	tmpz = _mm256_add_ps(tmpz, m32);

	/* Calculate the lower and upper bound. */
	float x[8], y[8], z[8];
	_mm256_store_ps(x, tmpx);
	_mm256_store_ps(y, tmpy);
	_mm256_store_ps(z, tmpz);
	Vector3 lower{ *x, *y, *z }, upper{ *x, *y, *z };
	for (size_t i = 1; i < 8; i++)
	{
		lower = min(lower, Vector3(x[i], y[i], z[i]));
		upper = max(upper, Vector3(x[i], y[i], z[i]));
	}

	/* Return the bounds with the OBB offset added. */
	return AABB(lower, upper);
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