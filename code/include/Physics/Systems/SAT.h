#pragma once
#include "Core/Math/Shapes/Plane.h"
#include "Core/Math/Shapes/Line.h"
#include "Core/Math/Shapes/OBB.h"

namespace Pu
{
	/* Defines a handler object for the Seperating-Axis-Theorem intersection algorithm. */
	class SAT
	{
	public:
		/* Gets the amount of SAT calls that occured since the last reset. */
		_Check_return_ static uint32 GetCallCount(void);
		/* Resets the SAT call counter. */
		static void ResetCounter(void);

		/* Performs SAT on the specified axis-aligned bounding box and specified oriented bounding box. */
		_Check_return_ bool Run(_In_ const AABB &aabb, _In_ const OBB &obb);
		/* Performs SAT on the two specified oriented bounding boxes. */
		_Check_return_ bool Run(_In_ const OBB &obb1, _In_ const OBB &obb2);
		/* Gets the contact points for the last collision [1, 4]. */
		_Check_return_ const vector<Vector3>& GetContacts(_In_ const OBB &obb1, _In_ const OBB &obb2);

		/* Gets the axis of intersection for the last SAT call. */
		_Check_return_ inline Vector3 GetIntersectionAxis(void) const
		{
			return n;
		}

		/* Gets the intersection depth for the last SAT call. */
		_Check_return_ inline float GetIntersectionDepth(void) const
		{
			return minDepth;
		}

	private:
		Vector3 c1[8], c2[8];
		Line l1[16], l2[16];
		Plane p1[6], p2[6];
		Vector3 axes[15];

		Vector3 n;
		float minDepth;
		vector<Vector3> contacts;

		static bool PlaneClipLine(Plane plane, Line line, Vector3 &result);
		static void FillBuffer(Plane *buffer, const OBB &obb);
		static void FillBuffer(Line *buffer, const Vector3 *corners);
		static void FillBuffer(Vector3 *buffer, Vector3 *axes, const AABB &aabb);
		static void FillBuffer(Vector3 *buffer, Vector3 *axes, const OBB &obb);

		void TransformAndCull(Vector3 p);
		bool RunInternal(void);
	};
}