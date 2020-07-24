#include "Core/Math/Shapes/OBB.h"
#include "Core/Math/Vector3_SIMD.h"

Pu::OBB Pu::OBB::operator*(const Matrix & transform) const
{
	return OBB(Center + transform.GetTranslation(), Extent * transform.GetScale(), Orientation * transform.GetOrientation());
}

Pu::AABB Pu::OBB::GetBoundingBox(void) const
{
	const ofloat two = _mm256_set1_ps(2.0f);

	/* Convert the corners to AVX. */
	const ofloat cx = _mm256_set_ps(-Extent.X, Extent.X, Extent.X, -Extent.X, -Extent.X, -Extent.X, Extent.X, Extent.X);
	const ofloat cy = _mm256_set_ps(-Extent.Y, -Extent.Y, Extent.Y, Extent.Y, Extent.Y, -Extent.Y, -Extent.Y, Extent.Y);
	const ofloat cz = _mm256_set_ps(-Extent.Z, -Extent.Z, -Extent.Z, -Extent.Z, Extent.Z, Extent.Z, Extent.Z, Extent.Z);

	/* Convert quaterion to AVX. */
	const ofloat qi = _mm256_set1_ps(Orientation.I);
	const ofloat qj = _mm256_set1_ps(Orientation.J);
	const ofloat qk = _mm256_set1_ps(Orientation.K);
	const ofloat qr = _mm256_set1_ps(Orientation.R);

	/* Orientation * Extent */
	const ofloat s1 = _mm256_mul_ps(two, _mm256_dot_v3(qi, qj, qk, cx, cy, cz));
	const ofloat s2 = _mm256_sub_ps(_mm256_mul_ps(qr, qr), _mm256_len2_v3(qi, qj, qk));
	const ofloat s3 = _mm256_mul_ps(two, qr);
	ofloat tmpx, tmpy, tmpz;
	_mm256_cross_v3(qi, qj, qk, cx, cy, cz, tmpx, tmpy, tmpz);
	tmpx = _mm256_add_ps(_mm256_mul_ps(s3, tmpx), _mm256_add_ps(_mm256_mul_ps(s1, qi), _mm256_mul_ps(s2, cx)));
	tmpy = _mm256_add_ps(_mm256_mul_ps(s3, tmpy), _mm256_add_ps(_mm256_mul_ps(s1, qj), _mm256_mul_ps(s2, cy)));
	tmpz = _mm256_add_ps(_mm256_mul_ps(s3, tmpz), _mm256_add_ps(_mm256_mul_ps(s1, qk), _mm256_mul_ps(s2, cz)));

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
	return AABB(Center + lower, Center + upper);
}