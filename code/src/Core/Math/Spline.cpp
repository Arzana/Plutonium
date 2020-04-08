#include "Core/Math/Spline.h"
#include "Core/Diagnostics/Logging.h"

Pu::Spline::Spline(void)
	: interpOp(Type::Cubic)
{}

Pu::Spline::Spline(Type interpolationOperation)
	: interpOp(interpolationOperation)
{}

Pu::SplineTransform Pu::Spline::GetNearest(float a) const
{
	const Factors values = GetStart(a, 2);
	return SplineTransform{ LocationNear(values), OrientationNear(values) };
}

Pu::SplineTransform Pu::Spline::GetLinear(float a) const
{
	const Factors values = GetStart(a, 2);
	return SplineTransform{ LocationLinear(values), OrientationLinear(values) };
}

Pu::SplineTransform Pu::Spline::GetCubic(float a) const
{
	const Factors values = GetStart(a, 3);
	return SplineTransform{ LocationCubic(values), OrientationCubic(values) };
}

Pu::SplineTransform Pu::Spline::GetQuadratic(float a) const
{
	const Factors values = GetStart(a, 4);
	return SplineTransform{ LocationQuadratic(values), OrientationQuadratic(values) };
}

Pu::SplineTransform Pu::Spline::Get(Type op, float a) const
{
	switch (op)
	{
	case Type::Nearest:
		return GetNearest(a);
	case Type::Linear:
		return GetLinear(a);
	case Type::Cubic:
		return GetCubic(a);
	case Type::Quadratic:
		return GetQuadratic(a);
	}

	return SplineTransform{};
}

Pu::Vector3 Pu::Spline::GetLocationNearest(float a) const
{
	return LocationNear(GetStart(a, 2));
}

Pu::Vector3 Pu::Spline::GetLocationLinear(float a) const
{
	return LocationLinear(GetStart(a, 2));
}

Pu::Vector3 Pu::Spline::GetLocationCubic(float a) const
{
	return LocationCubic(GetStart(a, 3));
}

Pu::Vector3 Pu::Spline::GetLocationQuadratic(float a) const
{
	return LocationQuadratic(GetStart(a, 4));
}

Pu::Vector3 Pu::Spline::GetLocation(Type op, float a) const
{
	switch (op)
	{
	case Type::Nearest:
		return GetLocationNearest(a);
	case Type::Linear:
		return GetLocationLinear(a);
	case Type::Cubic:
		return GetLocationCubic(a);
	case Type::Quadratic:
		return GetLocationQuadratic(a);
	}

	return Vector3{};
}

Pu::Quaternion Pu::Spline::GetOrientationNearest(float a) const
{
	return OrientationNear(GetStart(a, 2));
}

Pu::Quaternion Pu::Spline::GetOrientationLinear(float a) const
{
	return OrientationLinear(GetStart(a, 2));
}

Pu::Quaternion Pu::Spline::GetOrientationCubic(float a) const
{
	return OrientationCubic(GetStart(a, 2));
}

Pu::Quaternion Pu::Spline::GetOrientationQuadratic(float a) const
{
	return OrientationQuadratic(GetStart(a, 2));
}

Pu::Quaternion Pu::Spline::GetOrientation(Type op, float a) const
{
	switch (op)
	{
	case Type::Nearest:
		return GetOrientationNearest(a);
	case Type::Linear:
		return GetOrientationLinear(a);
	case Type::Cubic:
		return GetOrientationCubic(a);
	case Type::Quadratic:
		return GetOrientationQuadratic(a);
	}

	return Quaternion{};
}

Pu::Vector3 Pu::Spline::GetDirectionLinear(float a) const
{
	const Factors values = GetStart(a, 2);
	return dir(locations[values.first], locations[values.first + 1]);
}

Pu::Vector3 Pu::Spline::GetDirectionCubic(float a) const
{
	/* Get the bezier velocity (first derivative) and normalize it. */
	const Factors values = GetStart(a, 3);
	return (2.0f * (1.0f - values.second) * (locations[values.first + 1] - locations[values.first])
		+ 2.0f * values.second * (locations[values.first + 2] - locations[values.first + 1])).Normalize();
}

Pu::Vector3 Pu::Spline::GetDirectionQuadratic(float a) const
{
	/* Get the bezier velocity (first derivative) and normalize it. */
	const Factors values = GetStart(a, 4);
	const float ia = 1.0f - values.second;
	return (3.0f * sqr(ia) * (locations[values.first + 1] - locations[values.first])
		+ 6.0f * ia * values.second * (locations[values.first + 2] - locations[values.first + 1])
		+ 3.0f * sqr(values.second) * (locations[values.first + 3] - locations[values.first + 2])).Normalize();
}

Pu::Vector3 Pu::Spline::GetDirection(Type op, float a) const
{
	switch (op)
	{
	case Type::Linear:
		return GetDirectionLinear(a);
	case Type::Cubic:
		return GetDirectionCubic(a);
	case Type::Quadratic:
		return GetDirectionQuadratic(a);
	}

	return Vector3{};
}

void Pu::Spline::Add(Vector3 location, Quaternion orientation)
{
	locations.emplace_back(location);
	orientations.emplace_back(orientation);
}

Pu::Spline::Factors Pu::Spline::GetStart(float a, size_t ppt) const
{
	const size_t len = locations.size();
	if (a >= 1.0f)
	{
		/* Return the last possible start point when we've reached the end. */
		return std::make_pair(len - ppt, 1.0f);
	}

	/* 
	The normal start point is defined by the amount of points per type (ppt).
	And the interpolation between those points is just the fractional part of our input scale.
	*/
	a *= len - (ppt - 1);
	const size_t i = static_cast<size_t>(a);
	return std::make_pair(i, a - i);
}

Pu::Vector3 Pu::Spline::LocationNear(Factors factors) const
{
	return near(locations[factors.first], locations[factors.first + 1], factors.second);
}

Pu::Vector3 Pu::Spline::LocationLinear(Factors factors) const
{
	return lerp(locations[factors.first], locations[factors.first + 1], factors.second);
}

Pu::Vector3 Pu::Spline::LocationCubic(Factors factors) const
{
	return cubic(locations[factors.first], locations[factors.first + 1], locations[factors.first + 2], factors.second);
}

Pu::Vector3 Pu::Spline::LocationQuadratic(Factors factors) const
{
	return quadratic(locations[factors.first], locations[factors.first + 1], locations[factors.first + 2], locations[factors.first + 3], factors.second);
}

Pu::Quaternion Pu::Spline::OrientationNear(Factors factors) const
{
	return Quaternion::Near(orientations[factors.first], orientations[factors.first + 1], factors.second);
}

Pu::Quaternion Pu::Spline::OrientationLinear(Factors factors) const
{
	return Quaternion::SLerp(orientations[factors.first], orientations[factors.first + 1], factors.second);
}

Pu::Quaternion Pu::Spline::OrientationCubic(Factors factors) const
{
	//TODO: Add cubic spherical interpolation.
	return OrientationLinear(factors);
}

Pu::Quaternion Pu::Spline::OrientationQuadratic(Factors factors) const
{
	//TODO: Add quadratic spherical interpolation.
	return OrientationLinear(factors);
}
