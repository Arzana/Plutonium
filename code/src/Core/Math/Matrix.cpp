#include "Core\Math\Matrix.h"

using namespace Pu;

inline float det33(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
	return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
}

Pu::Matrix::operator string() const
{
	if (*this == Matrix()) return "[Identity]";

	string result;
	for (size_t c = 0; c < 4; c++)
	{
		for (size_t r = 0; r < 3; r++)
		{
			result += std::_Floating_to_string("%.2f", f[c * 4 + r]);
			result += ", ";
		}

		result += std::_Floating_to_string("%.2f", f[c * 4 + 3]);
		result += '\n';
	}

	return result;
}

Matrix Pu::Matrix::CreateRotation(float theta, Vector3 axis)
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
	float e = xy * omc + axis.Z * ss;
	float f = yy * omc + cc;
	float g = yz * omc - axis.X * ss;
	float i = xz * omc - axis.Y * ss;
	float j = yz * omc + axis.X * ss;
	float k = zz * omc + cc;

	return Matrix(a, b, c, 0.0f, e, f, g, 0.0f, i, j, k, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Pu::Matrix::CreateRotation(Quaternion quaternion)
{
	const float ii = sqr(quaternion.i);
	const float jj = sqr(quaternion.j);
	const float kk = sqr(quaternion.k);
	const float ir = quaternion.i * quaternion.r;
	const float ij = quaternion.i * quaternion.j;
	const float kr = quaternion.k * quaternion.r;
	const float ki = quaternion.k * quaternion.i;
	const float jr = quaternion.j * quaternion.r;
	const float jk = quaternion.j * quaternion.k;

	const float a = 1.0f - 2.0f * (jj + kk);
	const float b = 2.0f * (ij - kr);
	const float c = 2.0f * (ki + jr);
	const float e = 2.0f * (ij + kr);
	const float f = 1.0f - 2.0f * (kk + ii);
	const float g = 2.0f * (jk - ir);
	const float i = 2.0f * (ki - jr);
	const float j = 2.0f * (jk + ir);
	const float k = 1.0f - 2.0f * (jj + ii);

	return Matrix(a, b, c, 0.0f, e, f, g, 0.0f, i, j, k, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Pu::Matrix::CreateOrtho(float width, float height, float near, float far)
{
	const float a = 2.0f / width;
	const float f = 2.0f / height;
	const float k = 2.0f / (far - near);

	return Matrix(
		a, 0.0f, 0.0f, -1.0f, 
		0.0f, f, 0.0f, -1.0f, 
		0.0f, 0.0f, k, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Pu::Matrix::CreateOrtho(float left, float right, float bottom, float top, float near, float far)
{
	const float a = 2.0f / (right - left);
	const float f = 2.0f / (top - bottom);
	const float k = 2.0f / (far - near);
	const float d = -(right + left) / (right - left);
	const float h = -(top + bottom) / (top - bottom);
	const float l = -(far + near) / (far - near);

	return Matrix(
		a, 0.0f, 0.0f, d, 
		0.0f, f, 0.0f, h, 
		0.0f, 0.0f, k, l, 
		0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Pu::Matrix::CreateFrustum(float left, float right, float bottom, float top, float near, float far)
{
	const float a = (2.0f * near) / (right - left);
	const float c = -((right + left) / (right - left));
	const float f = -(2.0f * near) / (top - bottom);
	const float g = -((bottom + top) / (top - bottom));
	const float k = (far + near) / (far - near);
	const float l = -(2.0f * far * near) / (far - near);

	return Matrix(
		a, 0.0f, c, 0.0f,
		0.0f, f, g, 0.0f,
		0.0f, 0.0f, k, l,
		0.0f, 0.0f, 1.0f, 0.0f);
}

Matrix Pu::Matrix::CreatePerspective(float fovY, float aspr, float near, float far)
{
	/*
	This simpification can be made over the frustum function because the frustum is generic.
	If the frustum is symetric (like with a perspective camera) we can use the formula.
	Then we can solve a bit further so we don't have to multiply and divide with near for a and b.
	*/
	const float f = -1.0f / tanf(fovY * 0.5f);
	const float a = -f / aspr;
	const float k = (far + near) / (far - near);
	const float l = -(2.0f * far * near) / (far - near);

	return Matrix(
		a, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, k, l,
		0.0f, 0.0f, 1.0f, 0.0f);
}

Matrix Pu::Matrix::CreateLookIn(Vector3 pos, Vector3 direction, Vector3 up)
{
	const Vector3 axisX = normalize(cross(up, direction));
	const Vector3 axisY = cross(direction, axisX);

	const float d = -dot(axisX, pos);
	const float h = -dot(axisY, pos);
	const float l = -dot(direction, pos);

	return Matrix(
		axisX.X, axisX.Y, axisX.Z, d,
		axisY.X, axisY.Y, axisY.Z, h,
		direction.X, direction.Y, direction.Z, l,
		0.0f, 0.0f, 0.0f, 1.0f);
}

/*
Ken Shoemake 
University of Pennsylvania
*/
Quaternion Pu::Matrix::GetOrientation(void) const
{
	/* Get normalizes orientation columns. */
	const Vector3 aei = normalize(GetRight());
	const Vector3 bfj = normalize(GetUp());
	const Vector3 cgk = normalize(GetForward());

	/* Protect against /0. */
	const float t = aei.X + bfj.Y + cgk.Z;
	float i, j, k, r, s;
	if (t >= 0)
	{
		/* Divide by r. */
		s = sqrtf(t + 1);
		r = 0.5f * s;
		s = 0.5f / s;
		i = (cgk.Y - bfj.Z) * s;
		j = (aei.Z - cgk.X) * s;
		k = (bfj.X - aei.Y) * s;
	}
	else if (aei.X > bfj.Y && aei.X > cgk.Z)
	{
		/* Divide by i. */
		s = sqrtf(1 + aei.X - bfj.Y - cgk.Z);
		i = s * 0.5f;
		s = 0.5f / s;
		j = (bfj.X + aei.Y) * s;
		k = (aei.Z + cgk.X) * s;
		r = (cgk.Y - bfj.Z) * s;
	}
	else if (bfj.Y > cgk.Z)
	{
		/* Divide by j. */
		s = sqrt(1.0f + bfj.Y - aei.X - cgk.Z);
		j = s * 0.5f;
		s = 0.5f / s;
		i = (bfj.X + aei.Y) * s;
		k = (cgk.Y + bfj.Z) * s;
		r = (aei.Z - cgk.X) * s;
	}
	else
	{
		/* Divide by k. */
		s = sqrt(1 + cgk.Z - aei.X - bfj.Y);
		k = s * 0.5f;
		s = 0.5f / s;
		i = (aei.Z + cgk.X) * s;
		j = (cgk.Y + bfj.Z) * s;
		r = (bfj.X - aei.Y) * s;
	}

	return Quaternion(r, i, j, k);
}

Vector3 Pu::Matrix::GetScale(void) const
{
	return Vector3(GetRight().Length(), GetUp().Length(), GetForward().Length());
}

Matrix Pu::Matrix::GetStatic(void) const
{
	/* Simply remove the translation. */
	const Vector3 r = GetRight();
	const Vector3 u = GetUp();
	const Vector3 b = GetForward();

	return Matrix(
		r.X, u.X, b.X, 0.0f,
		r.Y, u.Y, b.Y, 0.0f,
		r.Z, u.Z, b.Z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

float Pu::Matrix::GetDeterminant(void) const
{
	return
		f[0] * det33(f[5], f[9], f[13], f[6], f[10], f[14], f[7], f[11], f[15]) -
		f[1] * det33(f[1], f[9], f[13], f[2], f[10], f[14], f[3], f[11], f[15]) +
		f[2] * det33(f[1], f[5], f[13], f[2], f[6], f[14], f[3], f[7], f[15]) -
		f[3] * det33(f[1], f[5], f[9], f[2], f[6], f[10], f[3], f[7], f[11]);
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
Matrix Pu::Matrix::GetInverse(void) const
{
	/* Inline calculate the matrix of minors needed for the determinant to save performance. */
	const float a = det33(this->f[5], this->f[9], this->f[13], this->f[6], this->f[10], this->f[14], this->f[7], this->f[11], this->f[15]);
	const float e = det33(this->f[1], this->f[9], this->f[13], this->f[2], this->f[10], this->f[14], this->f[3], this->f[11], this->f[15]);
	const float i = det33(this->f[1], this->f[5], this->f[13], this->f[2], this->f[6], this->f[14], this->f[3], this->f[7], this->f[15]);
	const float m = det33(this->f[1], this->f[5], this->f[9], this->f[2], this->f[6], this->f[10], this->f[3], this->f[7], this->f[11]);

	/* Calculate determinant and if it's zero early out with an identity matrix. */
	float det = f[0] * a - f[4] * e + f[8] * i - f[12] * m;
	if (det == 0.0f) return Matrix();

	/* Calculate matrix of minors for the full matrix (inline transposed). */
	const float b = det33(this->f[4], this->f[8], this->f[12], this->f[6], this->f[10], this->f[14], this->f[7], this->f[11], this->f[15]);
	const float c = det33(this->f[4], this->f[8], this->f[12], this->f[5], this->f[9], this->f[13], this->f[7], this->f[11], this->f[15]);
	const float d = det33(this->f[4], this->f[8], this->f[12], this->f[5], this->f[9], this->f[13], this->f[6], this->f[10], this->f[14]);
	const float f = det33(this->f[0], this->f[8], this->f[12], this->f[2], this->f[10], this->f[14], this->f[3], this->f[11], this->f[15]);
	const float g = det33(this->f[0], this->f[8], this->f[12], this->f[1], this->f[9], this->f[13], this->f[3], this->f[11], this->f[15]);
	const float h = det33(this->f[0], this->f[8], this->f[12], this->f[1], this->f[9], this->f[13], this->f[2], this->f[10], this->f[14]);
	const float j = det33(this->f[0], this->f[4], this->f[12], this->f[2], this->f[6], this->f[14], this->f[3], this->f[7], this->f[15]);
	const float k = det33(this->f[0], this->f[4], this->f[12], this->f[1], this->f[5], this->f[13], this->f[3], this->f[7], this->f[15]);
	const float l = det33(this->f[0], this->f[4], this->f[12], this->f[1], this->f[5], this->f[13], this->f[2], this->f[6], this->f[14]);
	const float n = det33(this->f[0], this->f[4], this->f[8], this->f[2], this->f[6], this->f[10], this->f[3], this->f[7], this->f[11]);
	const float o = det33(this->f[0], this->f[4], this->f[8], this->f[1], this->f[5], this->f[9], this->f[3], this->f[7], this->f[11]);
	const float p = det33(this->f[0], this->f[4], this->f[8], this->f[1], this->f[5], this->f[9], this->f[2], this->f[6], this->f[10]);

	/* Construct the adjugate matrix by checkboarding the determinants. */
	const Matrix adj = Matrix(
		+a, -b, +c, -d,
		-e, +f, -g, +h,
		+i, -j, +k, -l,
		-m, +n, -o, +p);

	/* Return the adjugate matrix multiplied by the inverse determinant. */
	det = 1.0f / det;
	return Matrix(adj.c1 * det, adj.c2 * det, adj.c3 * det, adj.c4 * det);
}
#pragma warning(pop)

Matrix Pu::Matrix::GetTranspose(void) const
{
	return Matrix(c1.X, c1.Y, c1.Z, c1.W,
		c2.X, c2.Y, c2.Z, c2.W,
		c3.X, c3.Y, c3.Z, c3.W,
		c4.X, c4.Y, c4.Z, c4.W);
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Pu::Matrix::SetOrientation(float yaw, float pitch, float roll)
{
	const float cz = cosf(pitch);
	const float sz = sinf(pitch);
	const float cy = cosf(yaw);
	const float sy = sinf(yaw);
	const float cx = cosf(roll);
	const float sx = sinf(roll);

	const float a = cx * cy;
	const float b = cx * sy * sz - sx * cz;
	const float c = cx * sy * cz + sx * sz;
	const float e = sx * cy;
	const float f = sx * sy * sz + cx * cz;
	const float g = sx * sy * cz - cx * sz;
	const float i = -sy;
	const float j = cy * sz;
	const float k = cy * cz;

	c1 = Vector4(a, e, i, 0.0f) * GetRight().Length();
	c2 = Vector4(b, f, j, 0.0f) * GetUp().Length();
	c3 = Vector4(c, g, k, 0.0f) * GetForward().Length();
}
#pragma warning(pop)

void Pu::Matrix::SetScale(float v)
{
	c1 = normalize(c1) * v;
	c2 = normalize(c2) * v;
	c3 = normalize(c3) * v;
}

void Pu::Matrix::SetScale(float x, float y, float z)
{
	c1 = normalize(c1) * x;
	c2 = normalize(c2) * y;
	c3 = normalize(c3) * z;
}

void Pu::Matrix::SetScale(Vector3 v)
{
	c1 = normalize(c1) * v.X;
	c2 = normalize(c2) * v.Y;
	c3 = normalize(c3) * v.Z;
}