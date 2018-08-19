#include "Core\Math\Matrix.h"

using namespace Plutonium;

Matrix Plutonium::Matrix::operator+(const Matrix & m) const
{
	return Matrix(f[0] + m.f[0], f[4] + m.f[4], f[8] + m.f[8], f[12] + m.f[12],
				  f[1] + m.f[1], f[5] + m.f[5], f[9] + m.f[9], f[13] + m.f[13],
				  f[2] + m.f[2], f[6] + m.f[6], f[10] + m.f[10], f[14] + m.f[14],
				  f[3] + m.f[3], f[7] + m.f[7], f[11] + m.f[11], f[15] + m.f[15]);
}

Matrix Plutonium::Matrix::operator-(const Matrix & m) const
{
	return Matrix(f[0] - m.f[0], f[4] - m.f[4], f[8] - m.f[8], f[12] - m.f[12],
				  f[1] - m.f[1], f[5] - m.f[5], f[9] - m.f[9], f[13] - m.f[13],
				  f[2] - m.f[2], f[6] - m.f[6], f[10] - m.f[10], f[14] - m.f[14],
				  f[3] - m.f[3], f[7] - m.f[7], f[11] - m.f[11], f[15] - m.f[15]);
}

Matrix Plutonium::Matrix::operator*(const Matrix & m) const
{
	return Matrix(operator*(m.c1), operator*(m.c2), operator*(m.c3), operator*(m.c4));
}

Matrix Plutonium::Matrix::CreateRotation(float theta, Vector3 axis)
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

Matrix Plutonium::Matrix::CreateRotation(float yaw, float pitch, float roll)
{
	return CreateRotationX(pitch) * CreateRotationY(yaw) * CreateRotationZ(roll);
}

Matrix Plutonium::Matrix::CreateOrtho(float width, float height, float near, float far)
{
	const float a = 2.0f / width;
	const float f = 2.0f / height;
	const float k = 2.0f / (far - near);

	return Matrix(a, 0.0f, 0.0f, 0.0f, 0.0f, f, 0.0f, 0.0f, 0.0f, 0.0f, k, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Plutonium::Matrix::CreateOrtho(float left, float right, float bottom, float top, float near, float far)
{
	const float a = 2.0f / (right - left);
	const float f = 2.0f / (top - bottom);
	const float k = 2.0f / (far - near);
	const float l = -(far + near) / (far - near);
	const float d = -(right + left) / (right - left);
	const float h = -(top + bottom) / (top - bottom);

	return Matrix(a, 0.0f, 0.0f, d, 0.0f, f, 0.0f, h, 0.0f, 0.0f, k, l, 0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix Plutonium::Matrix::CreateFrustum(float left, float right, float bottom, float top, float near, float far)
{
	const float a = (2.0f * near) / (right - left);
	const float c = (right + left) / (right - left);
	const float f = (2.0f * near) / (top - bottom);
	const float g = (top + bottom) / (top - bottom);
	const float k = -(far + near) / (far - near);
	const float l = -(2.0f * far * near) / (far - near);

	return Matrix(a, 0.0f, c, 0.0f, 0.0f, f, g, 0.0f, 0.0f, 0.0f, k, l, 0.0f, 0.0f, -1.0f, 0.0f);
}

Matrix Plutonium::Matrix::CreatPerspective(float fovY, float aspr, float near, float far)
{
	const float t = tanf(fovY * 0.5f);
	const float a = 1.0f / (aspr * t);
	const float f = 1.0f / t;
	const float k = -(far + near) / (far - near);
	const float l = -(2.0f * far * near) / (far - near);

	return Matrix(a, 0.0f, 0.0f, 0.0f, 0.0f, f, 0.0f, 0.0f, 0.0f, 0.0f, k, l, 0.0f, 0.0f, -1.0f, 0.0f);
}

Matrix Plutonium::Matrix::CreateLookAt(Vector3 pos, Vector3 target, Vector3 up)
{
	const Vector3 axisZ = dir(pos, target);
	const Vector3 axisX = normalize(cross(up, axisZ));
	const Vector3 axisY = cross(axisZ, axisX);

	const float d = -dot(axisX, pos);
	const float h = -dot(axisY, pos);
	const float l = -dot(axisZ, pos);

	return Matrix(axisX.X, axisX.Y, axisX.Z, d,
				  axisY.X, axisY.Y, axisY.Z, h,
				  axisZ.X, axisZ.Y, axisZ.Z, l,
				  0.0f, 0.0f, 0.0f, 1.0f);
}

inline float det33(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
	return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
}

Matrix Plutonium::Matrix::GetOrientation(void) const
{
	/* Get normalizes orientation columns. */
	Vector3 rn = normalize(GetRight());
	Vector3 un = normalize(GetUp());
	Vector3 bn = normalize(GetBackward());

	return Matrix(
		rn.X, un.X, bn.X, 0.0f,
		rn.Y, un.Y, bn.Y, 0.0f,
		rn.Z, un.Z, bn.Z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Vector3 Plutonium::Matrix::GetScale(void) const
{
	return Vector3(GetRight().Length(), GetUp().Length(), GetBackward().Length());
}

Matrix Plutonium::Matrix::GetStatic(void) const
{
	/* Simply remove the translation. */
	Vector3 r = GetRight();
	Vector3 u = GetUp();
	Vector3 b = GetBackward();

	return Matrix(
		r.X, u.X, b.X, 0.0f,
		r.Y, u.Y, b.Y, 0.0f,
		r.Z, u.Z, b.Z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

float Plutonium::Matrix::GetDeterminant(void) const
{
	return
		f[0] * det33(f[5], f[9], f[13], f[6], f[10], f[14], f[7], f[11], f[15]) -
		f[1] * det33(f[1], f[9], f[13], f[2], f[10], f[14], f[3], f[11], f[15]) +
		f[2] * det33(f[1], f[5], f[13], f[2], f[6], f[14], f[3], f[7], f[15]) -
		f[3] * det33(f[1], f[5], f[9], f[2], f[6], f[10], f[3], f[7], f[11]);
}

Matrix Plutonium::Matrix::GetInverse(void) const
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
	Matrix adj = Matrix(
		+a, -b, +c, -d, 
		-e, +f, -g, +h, 
		+i, -j, +k, -l,
		-m, +n, -o, +p);

	/* Return the adjugate matrix multiplied by the inverse determinant. */
	det = 1.0f / det;
	return Matrix(adj.c1 * det, adj.c2 * det, adj.c3 * det, adj.c4 * det);
}

Matrix Plutonium::Matrix::GetTranspose(void) const
{
	return Matrix(c1.X, c1.Y, c1.Z, c1.W,
				  c2.X, c2.Y, c2.Z, c2.W,
				  c3.X, c3.Y, c3.Z, c3.W,
				  c4.X, c4.Y, c4.Z, c4.W);
}

void Plutonium::Matrix::SetOrientation(float yaw, float pitch, float roll)
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
	c3 = Vector4(c, g, k, 0.0f) * GetBackward().Length();
}

void Plutonium::Matrix::SetScale(float v)
{
	c1 = normalize(c1) * v;
	c2 = normalize(c2) * v;
	c3 = normalize(c3) * v;
}

void Plutonium::Matrix::SetScale(float x, float y, float z)
{
	c1 = normalize(c1) * x;
	c2 = normalize(c2) * y;
	c3 = normalize(c3) * z;
}

void Plutonium::Matrix::SetScale(Vector3 v)
{
	c1 = normalize(c1) * v.X;
	c2 = normalize(c2) * v.Y;
	c3 = normalize(c3) * v.Z;
}
