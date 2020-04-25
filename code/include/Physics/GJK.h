#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines a handler object for the Gilbert-Johnson-Keerthi distance algorithm. */
	class GJK
	{
	public:
		/* Defines a function that returns the location with the greatest dot product with the specified direction. */
		using Support_t = _Check_return_ Vector3(*)(_In_ Vector3 dir, _In_ const void *userParam);

		/* Initializes a new instance of the GJK helper object. */
		GJK(void);
		/* Copy constructor. */
		GJK(_In_ const GJK &value) = default;
		/* Move constructor. */
		GJK(_In_ GJK &&avlue) = default;

		/* Copy assignment. */
		_Check_return_ GJK& operator =(_In_ const GJK &other) = default;
		/* Move assignment. */
		_Check_return_ GJK& operator =(_In_ GJK &&other) = default;

		/* Gets the amount of GJK calls that occured since the last reset. */
		_Check_return_ static uint32 GetCallCount(void);
		/* Gets the average iterations that GJK took since the last reset. */
		_Check_return_ static uint32 GetAverageIterations(void);
		/* Resets the GJK call counter. */
		static void ResetCounters(void);
		/* Runs a generic version of the GJK algorithm for the two specified shapes and their support functions; returning whether the two shapes intersect. */
		_Check_return_ bool Run(_In_ const void *shape1, _In_ const void *shape2, _In_ Support_t support1, _In_ Support_t support2);

	private:
		uint8 iteration;
		float lastSqrDist;
		Vector3 supportDir;
		
		uint8 dimension;
		Vector3 simplex[4];
		float barycentricCoords[4];

		int RunInternal(const void *shape1, const void *shape2, Support_t support1, Support_t support2);
		void NormalizeBarycentricCoords(void);
		void NearestLine(void);
		void NearestTriangle(void);
		void NearestTetrahedron(void);
	};
}