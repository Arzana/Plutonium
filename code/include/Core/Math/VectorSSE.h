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

		/* Implicitly converts the SSE stream vector to a normal vector. */
		_Check_return_ inline operator Vector3(void) const
		{
			return Vector3(AVX_FLOAT_UNION{ X }.V[0], AVX_FLOAT_UNION{ Y }.V[0], AVX_FLOAT_UNION{ Z }.V[0]);
		}

		/* Gets the magnetude of the vector squared. */
		_Check_return_ inline ofloat LengthSquared(void) const
		{
			return _mm256_add_ps(_mm256_mul_ps(X, X), _mm256_add_ps(_mm256_mul_ps(Y, Y), _mm256_mul_ps(Z, Z)));
		}

		/* Gets the magnetude of the vector. */
		_Check_return_ inline ofloat Length(void) const
		{
			return _mm256_sqrt_ps(LengthSquared());
		}
	};
}