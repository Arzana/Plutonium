#pragma once
#include "Plane.h"
#include "Matrix.h"

namespace Plutonium
{
	/* Defines a frustum as 6 planes. */
	struct Frustum
	{
	public:
		/* The 6 sides of the frustum. */
		Plane Planes[6];

		/* Creates an empty instance of a frustum. */
		Frustum(void)
		{}

		/* Creates a frustum from a specified matrix. */
		Frustum(_In_ const Matrix &matrix)
		{
			Vector4 row1 = _CrtGetRow<0>(matrix);
			Vector4 row2 = _CrtGetRow<1>(matrix);
			Vector4 row3 = _CrtGetRow<2>(matrix);
			Vector4 row4 = _CrtGetRow<3>(matrix);

			Left() = Plane(row4 + row1);
			Right() = Plane(row4 - row1);
			Bottom() = Plane(row4 + row2);
			Top() = Plane(row4 - row2);
			Near() = Plane(row4 + row3);
			Far() = Plane(row4 - row3);
		}

		/* Gets the near side of the frustum. */
		_Check_return_ inline Plane& Near(void)
		{
			return Planes[0];
		}

		/* Gets the far side of the frustum. */
		_Check_return_ inline Plane& Far(void)
		{
			return Planes[1];
		}

		/* Gets the left side of the frustum. */
		_Check_return_ inline Plane& Left(void)
		{
			return Planes[2];
		}

		/* Gets the right side of the frustum. */
		_Check_return_ inline Plane& Right(void)
		{
			return Planes[3];
		}

		/* Gets the top side of the frustum. */
		_Check_return_ inline Plane& Top(void)
		{
			return Planes[4];
		}

		/* Gets the bottom side of the frustum. */
		_Check_return_ inline Plane& Bottom(void)
		{
			return Planes[5];
		}

		/* Checks whether the specified sphere is fully or partially inside of the frustum. */
		_Check_return_ inline bool IntersectionSphere(_In_ Vector3 center, _In_ float radius) const
		{
			/* Loop through all of the planes. */
			constexpr size_t CNT = sizeof(Planes) / sizeof(Plane);
			for (size_t i = 0; i < CNT; i++)
			{
				/* If it's outside the plane it's outside of the frustum so we can early out. */
				if (!Planes[i].IntersectionSphere(center, radius)) return false;
			}

			/* It's inside or touching every plane so it must be inside the frustum. */
			return true;
		}

		/* Checks whether the specified box is fully or partially inside of the frustum. */
		_Check_return_ inline bool IntersectionBox(_In_ Vector3 pos, _In_ Vector3 size) const
		{
			/* Loop through all of the planes. */
			constexpr size_t CNT = sizeof(Planes) / sizeof(Plane);
			for (size_t i = 0; i < CNT; i++)
			{
				/* If it's outside the plane it's outside of the frustum so we can early out. */
				if (!Planes[i].IntersectionBox(pos, size)) return false;
			}

			/* It's inside or touching every plane so it must be inside the frustum. */
			return true;
		}
	};
}