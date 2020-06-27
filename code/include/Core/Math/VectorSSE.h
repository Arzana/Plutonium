#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines an ease of use 3D vector type to use with SSE streams. */
	struct Vector3SSE
	{
		/* Defines the X component of the vector. */
		ofloat X;
		/* Defines the Y component of the vector. */
		ofloat Y;
		/* Defines the Z component of the vector. */
		ofloat Z;

		/* Initializes an empty instance of a SSE vector. */
		Vector3SSE(void)
			: X(_mm256_setzero_ps()), Y(_mm256_setzero_ps()), Z(_mm256_setzero_ps())
		{}

		/* Initializes a new instance of an SSE vector. */
		Vector3SSE(_In_ float value)
			: X(_mm256_set1_ps(value)), Y(_mm256_set1_ps(value)), Z(_mm256_set1_ps(value))
		{}

		/* Initializes a new instance of an SSE vector. */
		Vector3SSE(_In_ float x, _In_ float y, _In_ float z)
			: X(_mm256_set1_ps(x)), Y(_mm256_set1_ps(y)), Z(_mm256_set1_ps(z))
		{}

		/* Initializes a new instance of an SSE vector. */
		Vector3SSE(_In_ Vector3 value)
			: X(_mm256_set1_ps(value.X)), Y(_mm256_set1_ps(value.Y)), Z(_mm256_set1_ps(value.Z))
		{}
	};
}