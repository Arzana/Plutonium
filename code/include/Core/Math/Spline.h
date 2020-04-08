#pragma once
#include <Core/Math/Quaternion.h>
#include <Core/Math/Interpolation.h>

namespace Pu
{
	/* Defines a location/orientation pair used by the spline. */
	struct SplineTransform
	{
		/* The interpolated location on the spline. */
		Vector3 Location;
		/* The interpolated orientation on the spline. */
		Quaternion Orientation;

		/* Initializes an empty instance of a spline transform. */
		SplineTransform(void) = default;
		/* Initializes a new instance of a spline transform. */
		SplineTransform(_In_ Vector3 location, _In_ Quaternion orientation)
			: Location(location), Orientation(orientation)
		{}
	};

	/* Defines multiple curve segments defined by keyframes consisting of locations and orientations. */
	class Spline
	{
	public:
		/* Defines the types of interpolation that can be used with a spline. */
		enum class Type
		{
			/* Gets the nearest keyframe (no interpolation). */
			Nearest,
			/* Linear interpolation. */
			Linear,
			/* Cubic bezier (default). */
			Cubic,
			/* Quadratic bezier. */
			Quadratic
		};

		/* Initializes an empty instance of a spline. */
		Spline(void);
		/* Initializes a new instance of a spline with a specific default interpolation type. */
		Spline(_In_ Type interpolationOperation);
		/* Copy constructor. */
		Spline(_In_ const Spline &value) = default;
		/* Move assignment. */
		Spline(_In_ Spline &&value) = default;

		/* Copy assignment. */
		_Check_return_ Spline& operator =(_In_ const Spline &other) = default;
		/* Move assignment. */
		_Check_return_ Spline& operator =(_In_ Spline &&other) = default;

		/* Gets the point on the spline with nearest interpolation. */
		_Check_return_ SplineTransform GetNearest(_In_ float a) const;
		/* Gets the point on the spline with linear interpolation. */
		_Check_return_ SplineTransform GetLinear(_In_ float a) const;
		/* Gets the point on the spline with cubic interpolation. */
		_Check_return_ SplineTransform GetCubic(_In_ float a) const;
		/* Gets the point on the spline with quadratic interpolation. */
		_Check_return_ SplineTransform GetQuadratic(_In_ float a) const;
		/* Gets the point on the spline with the specified interpolation. */
		_Check_return_ SplineTransform Get(_In_ Type op, _In_ float a) const;
		/* Gets the point on the spline with the default interpolation. */
		_Check_return_ inline SplineTransform Get(_In_ float a) const
		{
			return Get(interpOp, a);
		}

		/* Gets a location on the spline with nearest interpolation. */
		_Check_return_ Vector3 GetLocationNearest(_In_ float a) const;
		/* Gets a location on the spline with linear interpolation. */
		_Check_return_ Vector3 GetLocationLinear(_In_ float a) const;
		/* Gets a location on the spline with cubic interpolation. */
		_Check_return_ Vector3 GetLocationCubic(_In_ float a) const;
		/* Gets a location on the spline with quadratic interpolation. */
		_Check_return_ Vector3 GetLocationQuadratic(_In_ float a) const;
		/* Gets a location on the spline with the specified interpolation. */
		_Check_return_ Vector3 GetLocation(_In_ Type op, _In_ float a) const;
		/* Gets a location on the spline with the default interpolation. */
		_Check_return_ inline Vector3 GetLocation(_In_ float a) const
		{
			return GetLocation(interpOp, a);
		}

		/* Gets an orientation on the spline with nearest interpolation. */
		_Check_return_ Quaternion GetOrientationNearest(_In_ float a) const;
		/* Gets an orientation on the spline with linear interpolation. */
		_Check_return_ Quaternion GetOrientationLinear(_In_ float a) const;
		/* Gets an orientation on the spline with cubic interpolation. */
		_Check_return_ Quaternion GetOrientationCubic(_In_ float a) const;
		/* Gets an orientation on the spline with quadratic interpolation. */
		_Check_return_ Quaternion GetOrientationQuadratic(_In_ float a) const;
		/* Gets an orientation on the spline with the specified interpolation. */
		_Check_return_ Quaternion GetOrientation(_In_ Type op, _In_ float a) const;
		/* Gets an orientation on the spline with the default interpolation. */
		_Check_return_ inline Quaternion GetOrientation(_In_ float a) const
		{
			return GetOrientation(interpOp, a);
		}

		/* Gets the normalized first derivative of the spline at the specified point (linear). */
		_Check_return_ Vector3 GetDirectionLinear(_In_ float a) const;
		/* Gets the normalized first derivative of the spline at the specified point (cubic). */
		_Check_return_ Vector3 GetDirectionCubic(_In_ float a) const;
		/* Gets the normalized first derivative of the spline at the specified point (quadratic). */
		_Check_return_ Vector3 GetDirectionQuadratic(_In_ float a) const;
		/* Gets the normalized first derivative of the spline at the specified point. */
		_Check_return_ Vector3 GetDirection(_In_ Type op, _In_ float a) const;
		/* Gets the normalized first derivative of the spline at the specified point (default). */
		_Check_return_ inline Vector3 GetDirection(_In_ float a) const
		{
			return GetDirection(interpOp, a);
		}

		/* Adds a new key frame to the spline. */
		void Add(_In_ Vector3 location, _In_ Quaternion orientation);
		/* Gets the amount of points in the spline. */
		_Check_return_ inline size_t Count(void) const
		{
			return locations.size();
		}

	private:
		using Factors = std::pair<size_t, float>;

		Type interpOp;
		vector<Vector3> locations;
		vector<Quaternion> orientations;

		Factors GetStart(float a, size_t ppt) const;

		Vector3 LocationNear(Factors factors) const;
		Vector3 LocationLinear(Factors factors) const;
		Vector3 LocationCubic(Factors factors) const;
		Vector3 LocationQuadratic(Factors factors) const;

		Quaternion OrientationNear(Factors factors) const;
		Quaternion OrientationLinear(Factors factors) const;
		Quaternion OrientationCubic(Factors factors) const;
		Quaternion OrientationQuadratic(Factors factors) const;
	};
}