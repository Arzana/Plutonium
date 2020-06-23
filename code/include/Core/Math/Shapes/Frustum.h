#pragma once
#include "Plane.h"

namespace Pu
{
	/* Defines a frustum as 6 planes. */
	struct Frustum
	{
	public:
		/* The 6 sides of the frustum. */
		Plane Planes[6];

		/* Creates an empty instance of a frustum. */
		Frustum(void) = default;

		/* Creates a frustum from a specified matrix. */
		Frustum(_In_ const Matrix &matrix)
		{
			const Vector4 row1 = _CrtGetRow<0>(matrix);
			const Vector4 row2 = _CrtGetRow<1>(matrix);
			const Vector4 row3 = _CrtGetRow<2>(matrix);
			const Vector4 row4 = _CrtGetRow<3>(matrix);

			Planes[2] = Plane(row4 + row1);
			Planes[3] = Plane(row4 - row1);
			Planes[5] = Plane(row4 + row2);
			Planes[4] = Plane(row4 - row2);
			Planes[0] = Plane(row4 + row3);
			Planes[1] = Plane(row4 - row3);
		}

		/* Gets the near side of the frustum. */
		_Check_return_ inline const Plane& Near(void) const
		{
			return Planes[0];
		}

		/* Gets the far side of the frustum. */
		_Check_return_ inline const Plane& Far(void) const
		{
			return Planes[1];
		}

		/* Gets the left side of the frustum. */
		_Check_return_ inline const Plane& Left(void) const
		{
			return Planes[2];
		}

		/* Gets the right side of the frustum. */
		_Check_return_ inline const Plane& Right(void) const
		{
			return Planes[3];
		}

		/* Gets the top side of the frustum. */
		_Check_return_ inline const Plane& Top(void) const
		{
			return Planes[4];
		}

		/* Gets the bottom side of the frustum. */
		_Check_return_ inline const Plane& Bottom(void) const
		{
			return Planes[5];
		}
	};
}