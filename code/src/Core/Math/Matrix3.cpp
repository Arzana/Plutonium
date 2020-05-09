#include "Core/Math/Matrix3.h"

inline float det22(float a, float b, float c, float d)
{
	return a * d - b * c;
}

Pu::Matrix3 Pu::Matrix3::CreateRotation(float theta, Vector3 axis)
{
	const float cc = cosf(theta);
	const float ss = sinf(theta);
	const float xx = axis.X * axis.X;
	const float xy = axis.X * axis.Y;
	const float xz = axis.X * axis.Z;
	const float yy = axis.Y * axis.Y;
	const float yz = axis.Y * axis.Z;
	const float zz = axis.Z * axis.Z;
	const float omc = 1.0f - cc;

	float a = xx * omc + cc;
	float b = xy * omc - axis.Z * ss;
	float c = xz * omc + axis.Y * ss;
	float d = xy * omc + axis.Z * ss;
	float e = yy * omc + cc;
	float f = yz * omc - axis.X * ss;
	float g = xz * omc - axis.Y * ss;
	float h = yz * omc + axis.X * ss;
	float i = zz * omc + cc;

	return Matrix3(a, b, c, d, e, f, g, h, i);
}

Pu::Matrix3 Pu::Matrix3::CreateRotation(Quaternion quaternion)
{
	const float ii = sqr(quaternion.I);
	const float jj = sqr(quaternion.J);
	const float kk = sqr(quaternion.K);
	const float ir = quaternion.I * quaternion.R;
	const float ij = quaternion.I * quaternion.J;
	const float kr = quaternion.K * quaternion.R;
	const float ki = quaternion.K * quaternion.I;
	const float jr = quaternion.J * quaternion.R;
	const float jk = quaternion.J * quaternion.K;

	const float a = 1.0f - 2.0f * (jj + kk);
	const float b = 2.0f * (ij - kr);
	const float c = 2.0f * (ki + jr);
	const float d = 2.0f * (ij + kr);
	const float e = 1.0f - 2.0f * (kk + ii);
	const float f = 2.0f * (jk - ir);
	const float g = 2.0f * (ki - jr);
	const float h = 2.0f * (jk + ir);
	const float i = 1.0f - 2.0f * (jj + ii);

	return Matrix3(a, b, c, d, e, f, g, h, i);
}

float Pu::Matrix3::GetDeterminant(void) const
{
	return f[0] * f[4] * f[8] + f[3] * f[7] * f[2] + f[6] * f[1] * f[5] - f[6] * f[4] * f[2] - f[3] * f[1] * f[8] - f[0] * f[7] * f[5];
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
Pu::Matrix3 Pu::Matrix3::GetInverse(void) const
{
	/* Inline calculate the matrix of minors needed for the determinant to save performance. */
	const float a = det22(this->f[4], this->f[7], this->f[5], this->f[8]);
	const float d = det22(this->f[1], this->f[7], this->f[2], this->f[8]);
	const float g = det22(this->f[1], this->f[4], this->f[2], this->f[5]);

	/* Calculate determinant and if it's zero early out with an identity matrix. */
	const float det = f[0] * a - f[1] * d + f[6] * g;
	if (det == 0.0f) return Matrix3();

	/* Calculate matrix of minors for the full matrix (inline transposed). */
	const float b = det22(this->f[3], this->f[6], this->f[5], this->f[8]);
	const float c = det22(this->f[3], this->f[6], this->f[4], this->f[7]);
	const float e = det22(this->f[0], this->f[6], this->f[2], this->f[8]);
	const float f = det22(this->f[0], this->f[6], this->f[1], this->f[7]);
	const float h = det22(this->f[0], this->f[3], this->f[2], this->f[5]);
	const float i = det22(this->f[0], this->f[3], this->f[1], this->f[4]);

	/* Construct the adjugate matrix by checkboarding the determinants. */
	const Matrix3 adj = Matrix3(
		+a, -b, +c,
		-d, +e, -f,
		+g, -h, +i);

	/* Return the adjugate matrix multiplied by the inverse determinant. */
	return adj * recip(det);
}
#pragma warning (pop)

Pu::Matrix3 Pu::Matrix3::GetTranspose(void) const
{
	return Matrix3(c1.X, c1.Y, c1.Z,
		c2.X, c2.Y, c2.Z,
		c3.Z, c3.Y, c3.Z);
}

Pu::string Pu::Matrix3::ToString(void) const
{
	if (nrlyeql(GetDeterminant(), 1.0f)) return "[Identity]";

	string result;
	for (size_t c = 0; c < 3; c++)
	{
		for (size_t r = 0; r < 2; r++)
		{
			result += std::_Floating_to_string("%.2f", f[c * 3 + r]);
			result += ", ";
		}

		result += std::_Floating_to_string("%.2f", f[c * 3 + 2]);
		result += '\n';
	}

	return result;
}
