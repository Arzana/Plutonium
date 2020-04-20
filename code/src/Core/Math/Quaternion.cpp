#include "Core/Math/Quaternion.h"

Pu::Quaternion Pu::Quaternion::operator*(Quaternion q) const
{
	return Quaternion(
		r * q.r - (i * q.i + j * q.j + k * q.k),
		i * q.r + q.i * r + (j * q.k - k * q.j),
		j * q.r + q.j * r + (k * q.i - i * q.k),
		k * q.r + q.k * r + (i * q.j - j * q.i));
}

Pu::Vector3 Pu::Quaternion::operator*(Vector3 v) const
{
	const Vector3 u{ i, j, k };
	const Vector3 x = 2.0f * dot(u, v) * u;
	const Vector3 y = (r * r - u.LengthSquared()) * v;
	const Vector3 z = 2.0f * r * cross(u, v);
	return x + y + z;
}

/*
Ken Shoemake
University of Pennsylvania
*/
Pu::Quaternion Pu::Quaternion::Create(Vector3 forward, Vector3 up)
{
	const Vector3 axisX = normalize(cross(up, forward));
	const Vector3 axisY = cross(forward, axisX);
	const float t = axisX.X + axisY.Y + forward.Z;

	float r, i, j, k, l;
	if (t > 0.0f)
	{
		l = sqrtf(t + 1.0f);
		r = l * 0.5f;
		l = 0.5f / l;

		i = (axisY.Z - forward.Y) * l;
		j = (forward.X - axisX.Z) * l;
		k = (axisX.Y - axisY.X) * l;
	}
	else if (axisX.X >= axisY.Y && axisX.X >= forward.Z)
	{
		l = sqrtf(1.0f + axisX.X - axisY.Y - forward.Z);
		i = l * 0.5f;
		l = 0.5f / l;
		
		j = (axisX.Y + axisY.X) * l;
		k = (axisX.Z + forward.X) * l;
		r = (axisY.Z - forward.Y) * l;
	}
	else if (axisY.Y > forward.Z)
	{
		l = sqrtf(1.0f + axisY.Y - axisX.X - forward.Z);
		j = l * 0.5f;
		l = 0.5f / l;

		i = (axisY.X + axisX.Y) * l;
		k = (forward.Y + axisY.Z) * l;
		r = (forward.X - axisX.Z) * l;
	}
	else
	{
		l = sqrtf(1.0f + forward.Z - axisX.X - axisY.Y);
		k = 0.5f * l;
		l = 0.5f / l;

		i = (forward.X + axisX.Z) * l;
		j = (forward.Y + axisY.Z) * l;
		r = (axisX.Y - axisY.X) * l;
	}

	return Quaternion(r, i, j, k);
}

Pu::Quaternion Pu::Quaternion::Create(float theta, Vector3 axis)
{
	theta *= 0.5f;
	const float s = sinf(theta);
	return Quaternion(cosf(theta), axis.X * s, axis.Y * s, axis.Z * s);
}

/*
This method works by creating 3 quaternions (for yaw, pitch and roll) and the multiplying them.
The creation of the quaternions and their multiplication is inlined in order to perform less multiplications.
*/
Pu::Quaternion Pu::Quaternion::Create(float yaw, float pitch, float roll)
{
	yaw *= 0.5f;
	pitch *= 0.5f;
	roll *= 0.5f;

	const float sy = sinf(yaw);
	const float cy = cosf(yaw);
	const float sp = sinf(pitch);
	const float cp = cosf(pitch);
	const float sr = sinf(roll);
	const float cr = cosf(roll);

	return Quaternion(
		cy * cp * cr + sy * sp * sr,
		cy * sp * cr + sy * cp * sr,
		sy * cp * cr - cy * sp * sr,
		cy * cp * sr - sy * sp * cr);
}

Pu::Quaternion Pu::Quaternion::Delta(Quaternion q1, Quaternion q2)
{
	return q1 * q2.Inverse();
}

Pu::Quaternion Pu::Quaternion::Near(Quaternion q1, Quaternion q2, float a)
{
	return a < 0.5f ? q1 : q2;
}

Pu::Quaternion Pu::Quaternion::Lerp(Quaternion q1, Quaternion q2, float a)
{
	/* Split the amount into two values, a and ia where a + ia = 1.0f. */
	const float ia = 1.0f - a;

	/* Check if the quaternions are pointing in the right direction and perform interpolation. */
	Quaternion result = dot(q1, q2) >= 0.0f ? (q1 * ia + q2 * a) : (q1 * ia - q2 * a);

	/* Return the normalized result. */
	return result.Normalize();
}

Pu::Quaternion Pu::Quaternion::SLerp(Quaternion q1, Quaternion q2, float a)
{
	constexpr float THRESHOLD = 0.9995f;

	/* Check if the quaternions are pointing in the right direction, if not invert them. */
	float a1, a2;
	float d = dot(q1, q2);
	bool negated = false;
	if (d < 0.0f)
	{
		negated = true;
		d = -d;
	}

	/* If the quaternions are very close we can just do a normal lerp. */
	if (d > THRESHOLD)
	{
		a1 = 1.0f - a;
		a2 = negated ? -a : a;
	}
	else
	{
		/* Compute amount values for both. */
		const float theta = acosf(d);
		const float isin = recip(sinf(theta));
		a1 = sinf((1.0f - a) * theta) * isin;
		a2 = negated ? (-sinf(a * theta) * isin) : (sinf(a * theta) * isin);
	}

	/* Return new quaternion. */
	return q1 * a1 + q2 * a2;
}

Pu::Quaternion Pu::Quaternion::CLerp(Quaternion q1, Quaternion q2, Quaternion q3, float a)
{
	const float b = 2.0f * a * (1.0f - a);
	return Quaternion::SLerp(Quaternion::SLerp(q1, q2, a), Quaternion::SLerp(q1, q3, a), b);
}

Pu::Quaternion Pu::Quaternion::Unpack(int64 packed)
{
	/* [kkkkkkkkkkkkkkkkkkkkkjjjjjjjjjjjjjjjjjjjjjiiiiiiiiiiiiiiiiiiiii0] */
	const float i = (packed >> 42) * 0.00000047683716f;
	const float j = ((packed << 22) >> 43) * 0.00000095367432f;
	const float k = (packed & 0x1FFFFF) * 0.00000095367432f;

	/* 
	Make sure that we handle the rotation where the real component would be zero. 
	[-116, 42, -116]
	*/
	const float ilen = 1.0f - (i * i + j * j + k * k);
	const float r = ilen >= 0.00000095367432f ? sqrtf(ilen) : 0.0f;

	return Quaternion{ r, i, j, k };
}

Pu::Quaternion Pu::Quaternion::Inverse(void) const
{
	/* Inverse is just the quaterion conjugate divided by its squared magnitude. */
	return Quaternion(r, -i, -j, -k) * recip(LengthSquared());
}

Pu::int64 Pu::Quaternion::Pack(void) const
{
	/*
	Packs the quaterion as a normalized vector with 21 bits of precision per component.
	The real component can be recalculated when unpacking.
	*/
	const float sign = r >= 0.0f ? 1.0f : -1.0f;
	const int64 x = static_cast<int64>(sign * i * 2097152.0f) << 21;
	const int64 y = static_cast<int64>(sign * j * 1048576.0f) & 0x1FFFFF;
	const int64 z = static_cast<int64>(sign * k * 1048576.0f) & 0x1FFFFF;
	return z | ((y | x) << 21);
}