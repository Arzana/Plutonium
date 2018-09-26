#include "Core\Math\Quaternion.h"

using namespace Plutonium;

Quaternion Plutonium::Quaternion::operator*(Quaternion q) const
{
	return Quaternion(
		I * q.R + q.I * R + (J * q.K - K * q.J),
		J * q.R + q.J * R + (K * q.I - I * q.K),
		K * q.R + q.K * R + (I * q.J - J * q.I),
		R * q.R - (I * q.I + J * q.J + K * q.K));
}

Quaternion Plutonium::Quaternion::operator/(Quaternion q) const
{
	const float il2 = recip(q.LengthSquared());
	const float ii = -q.I * il2;
	const float ij = -q.J * il2;
	const float ik = -q.K * il2;
	const float sr = q.R * il2;

	return Quaternion(
		I * sr + ii * R + J * ik - K * ij,
		J * sr + ij * R + K * ii - I * ik,
		K * sr + ik * R + I * ij - J * ii,
		R * sr - (I * ii + J * ij) + K * ik);
}

Quaternion Plutonium::Quaternion::CreateRotation(float theta, Vector3 axis)
{
	theta *= 0.5f;
	const float s = sinf(theta);
	return Quaternion(axis.X * s, axis.Y * s, axis.Z * s, cosf(theta));
}

Quaternion Plutonium::Quaternion::CreateRotation(float yaw, float pitch, float roll)
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
		cy * sp * cr + sy * cp * sr,
		sy * cp * cr - cy * sp * sr,
		cy * cp * sr - sy * sp * cr,
		cy * cp * cr + sy * sp * sr);
}

Quaternion Plutonium::Quaternion::SLerp(Quaternion q1, Quaternion q2, float a)
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