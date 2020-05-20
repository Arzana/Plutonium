#include "Physics/Systems/GJK.h"
#include "Core/Diagnostics/Logging.h"
#include "Config.h"

#define GJK_EXIT_SUCCESS	1
#define GJK_EXIT_FAILED		0
#define GJK_EXIT_ERROR		-1

static Pu::uint32 calls = 0;
static Pu::uint32 iterations = 0;

Pu::GJK::GJK(void)
	: iteration(0), lastSqrDist(maxv<float>()), dimension(0)
{
	memset(barycentricCoords, 0, sizeof(barycentricCoords));
}

Pu::uint32 Pu::GJK::GetCallCount(void)
{
	return calls;
}

Pu::uint32 Pu::GJK::GetAverageIterations(void)
{
	return iterations / max(1u, calls);
}

void Pu::GJK::ResetCounters(void)
{
	calls = 0;
	iterations = 0;
}

bool Pu::GJK::Run(const void * shape1, const void * shape2, Support_t support1, Support_t support2)
{
	return RunInternal(shape1, shape2, support1, support2) == 1;
}

int Pu::GJK::RunInternal(const void * shape1, const void * shape2, Support_t support1, Support_t support2)
{
	++calls;

	/* Guess the starting direction for the algorithm */
	Vector3 a = support1(Vector3::Left(), shape1) - support2(Vector3::Right(), shape2);
	supportDir = -a;

	/* Initialize the simplex. */
	dimension = 1;
	simplex[0] = a;
	barycentricCoords[0] = 1.0f;

	/* Loop untill we run out of iterations (should not occur often). */
	for (iteration = 0; iteration < MaxIterationsGJK; iteration++)
	{
		iterations++;

		/* Gets the new support point. */
		a = support1(supportDir, shape1) - support2(-supportDir, shape2);

		/* Add the new location to the simplex. */
		simplex[dimension] = a;
		barycentricCoords[dimension++] = 1.0f;

		/* Solve for the new simplex. */
		switch (dimension)
		{
		case 1:
			break;
		case 2:
			NearestLine();
			break;
		case 3:
			NearestTriangle();
			break;
		case 4:
			NearestTetrahedron();
			break;
		default:
			Log::Error("GJK failed (Simplex of dimension %u cannot be solved)!", dimension);
			return GJK_EXIT_ERROR;
		}

		/* The origin is in the simplex if it remains a tetrahedron after solving. */
		if (dimension == 4) return GJK_EXIT_SUCCESS;

		NormalizeBarycentricCoords();

		/* Calculate the closest point. */
		Vector3 p;
		switch (dimension)
		{
		case 1:
			p = simplex[0];
			break;
		case 2:
			p = simplex[0] * barycentricCoords[0] + simplex[1] * barycentricCoords[1];
			break;
		case 3:
			p = simplex[1] * barycentricCoords[0] + simplex[1] * barycentricCoords[1] + simplex[2] * barycentricCoords[2];
			break;
		}

		/* Ensure that we're closing in on the origin, to prevent multi-step cycling. */
		const float d2 = p.LengthSquared();
		if (d2 == 0.0f) return GJK_EXIT_SUCCESS;
		if (d2 >= lastSqrDist) break;
		lastSqrDist = d2;

		/* Get the new search direction. */
		switch (dimension)
		{
		case 1:
			supportDir = -simplex[0];
			break;
		case 2:
		{
			Vector3 ba = simplex[1] - simplex[0];
			supportDir = cross(cross(ba, -simplex[1]), ba);
			break;
		}
		case 3:
			Vector3 n = cross(simplex[1] - simplex[0], simplex[2] - simplex[0]);
			supportDir = dot(n, simplex[0]) <= 0.0f ? n : -n;
			break;
		}

		/* Something went wrong and the direction is invalid. */
		if (supportDir.LengthSquared() < sqr(EPSILON))
		{
			Log::Error("GJK failed (Search direction is [0, 0, 0])!");
			return GJK_EXIT_ERROR;
		}
	}

	/* We ran out of iterations, so return failed. */
	return GJK_EXIT_FAILED;
}

void Pu::GJK::NormalizeBarycentricCoords(void)
{
	/* Normalize the barycentric coordinates to [0, 1] range. */
	float denom = 0.0f;
	for (uint8 i = 0; i < dimension; i++) denom += barycentricCoords[i];
	denom = recip(denom);

	for (uint8 i = 0; i < dimension; i++) barycentricCoords[i] *= denom;
}

void Pu::GJK::NearestLine(void)
{
	/* Calculate the unnormalized barycentric coordinates for the line. */
	const Vector3 ab = simplex[0] - simplex[1];
	const Vector3 ba = simplex[1] - simplex[0];
	const float u = dot(simplex[1], ba);
	const float v = dot(simplex[0], ab);

	if (v <= 0.0f)
	{
		/* Vertex region A. */
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (u <= 0.0f)
	{
		/* Vertex region B. */
		simplex[0] = simplex[1];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else
	{
		/* Line region AB. */
		barycentricCoords[0] = u;
		barycentricCoords[1] = v;
		dimension = 2;
	}
}

void Pu::GJK::NearestTriangle(void)
{
	/* Get the unnormalized barycentric coordinates for the line segments in the triangle. */
	const Vector3 ab = simplex[0] - simplex[1];
	const Vector3 ba = simplex[1] - simplex[0];
	const Vector3 bc = simplex[1] - simplex[2];
	const Vector3 cb = simplex[2] - simplex[1];
	const Vector3 ca = simplex[2] - simplex[0];
	const Vector3 ac = simplex[0] - simplex[2];

	const float uab = dot(simplex[1], ba);
	const float vab = dot(simplex[0], ab);
	const float ubc = dot(simplex[2], cb);
	const float vbc = dot(simplex[1], bc);
	const float uca = dot(simplex[0], ac);
	const float vca = dot(simplex[2], ca);

	if (vab <= 0.0f && uca <= 0.0f)
	{
		/* Vertex region A. */
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (uab < 0.0f && vbc <= 0.0f)
	{
		/* Vertex region B. */
		simplex[0] = simplex[1];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (ubc <= 0.0f && vca <= 0.0f)
	{
		/* Vertex region C. */
		simplex[0] = simplex[2];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else
	{
		/* Calculate the area of the face regions. */
		const Vector3 n = cross(ba, ca);
		const float uabc = box(n, simplex[1], simplex[2]);
		const float vabc = box(n, simplex[2], simplex[0]);
		const float wabc = box(n, simplex[0], simplex[1]);

		/* The point is not in one of the vertex regions, so move on to the next highest feature, lines, and then the face. */
		if (uab > 0.0f && vab > 0.0f && wabc <= 0.0f)
		{
			/* Region AB. */
			barycentricCoords[0] = uab;
			barycentricCoords[1] = vab;
			dimension = 2;
		}
		else if (ubc > 0.0f && vbc > 0.0f && uabc <= 0.0f)
		{
			/* Region BC. */
			simplex[0] = simplex[1];
			simplex[1] = simplex[2];
			barycentricCoords[0] = ubc;
			barycentricCoords[1] = vbc;
			dimension = 2;
		}
		else if (uca > 0.0f && vca > 0.0f && vabc <= 0.0f)
		{
			/* Region CA. */
			simplex[1] = simplex[0];
			simplex[0] = simplex[2];
			barycentricCoords[0] = uca;
			barycentricCoords[1] = vca;
			dimension = 2;
		}
		else
		{
			/* Face region ABC. */
			barycentricCoords[0] = uabc;
			barycentricCoords[1] = vabc;
			barycentricCoords[2] = wabc;
			dimension = 3;
		}
	}
}

void Pu::GJK::NearestTetrahedron(void)
{
	/* Get the unnormalized barycentric coordinates for the line segments in the triangle. */
	const Vector3 ab = simplex[0] - simplex[1];
	const Vector3 ba = simplex[1] - simplex[0];
	const Vector3 bc = simplex[1] - simplex[2];
	const Vector3 cb = simplex[2] - simplex[1];
	const Vector3 ca = simplex[2] - simplex[0];
	const Vector3 ac = simplex[0] - simplex[2];
	const Vector3 db = simplex[3] - simplex[1];
	const Vector3 bd = simplex[1] - simplex[3];
	const Vector3 dc = simplex[3] - simplex[2];
	const Vector3 cd = simplex[2] - simplex[3];
	const Vector3 da = simplex[3] - simplex[0];
	const Vector3 ad = simplex[0] - simplex[3];

	const float uab = dot(simplex[1], ba);
	const float vab = dot(simplex[0], ab);
	const float ubc = dot(simplex[2], cb);
	const float vbc = dot(simplex[1], bc);
	const float uca = dot(simplex[0], ac);
	const float vca = dot(simplex[2], ca);
	const float ubd = dot(simplex[3], db);
	const float vbd = dot(simplex[1], bd);
	const float udc = dot(simplex[2], cd);
	const float vdc = dot(simplex[3], dc);
	const float uad = dot(simplex[3], da);
	const float vad = dot(simplex[0], ad);

	if (vab <= 0.0f && uca <= 0.0f && vad <= 0.0f)
	{
		/* Vertex region A. */
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (uab <= 0.0f && vbc <= 0.0f && vbd <= 0.0f)
	{
		/* Vertex region B. */
		simplex[0] = simplex[1];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (ubc <= 0.0f && vca <= 0.0f && udc <= 0.0f)
	{
		/* Vertex region C. */
		simplex[0] = simplex[2];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else if (ubd <= 0.0f && vdc <= 0.0f && uad <= 0.0f)
	{
		/* Vertex region D. */
		simplex[0] = simplex[3];
		barycentricCoords[0] = 1.0f;
		dimension = 1;
	}
	else
	{
		/* Calculate the area of the face regions. */
		Vector3 n = cross(da, ba);
		const float uadb = box(n, simplex[3], simplex[1]);
		const float vadb = box(n, simplex[1], simplex[0]);
		const float wadb = box(n, simplex[0], simplex[3]);

		n = cross(ca, da);
		const float uacd = box(n, simplex[2], simplex[3]);
		const float vacd = box(n, simplex[3], simplex[0]);
		const float wacd = box(n, simplex[0], simplex[2]);

		n = cross(bc, dc);
		const float ucbd = box(n, simplex[1], simplex[3]);
		const float vcbd = box(n, simplex[3], simplex[2]);
		const float wcbd = box(n, simplex[2], simplex[3]);

		n = cross(ba, ca);
		const float uabc = box(n, simplex[1], simplex[2]);
		const float vabc = box(n, simplex[2], simplex[0]);
		const float wabc = box(n, simplex[0], simplex[1]);

		/* The location was not in a vertex region so we move on to test line regions. */
		if (wabc <= 0.0f && vadb <= 0.0f && uab > 0.0f && vab > 0.0f)
		{
			/* Line region AB. */
			barycentricCoords[0] = uab;
			barycentricCoords[1] = vab;
			dimension = 2;
		}
		else if (uabc <= 0.0f && wcbd <= 0.0f && ubc > 0.0f && vbc > 0.0f)
		{
			/* Line region BC. */
			simplex[0] = simplex[1];
			simplex[1] = simplex[2];
			barycentricCoords[0] = ubc;
			barycentricCoords[1] = vbc;
			dimension = 2;
		}
		else if (vabc <= 0.0f && wacd <= 0.0f && uca > 0.0f && vca > 0.0f)
		{
			/* Line region CA. */
			simplex[1] = simplex[0];
			simplex[0] = simplex[2];
			barycentricCoords[0] = uca;
			barycentricCoords[1] = vca;
			dimension = 2;
		}
		else if (vcbd <= 0.0f && uacd <= 0.0f && udc > 0.0f && vdc > 0.0f)
		{
			/* Line region DC. */
			simplex[0] = simplex[3];
			simplex[1] = simplex[2];
			barycentricCoords[0] = uad;
			barycentricCoords[1] = vad;
			dimension = 2;
		}
		else if (vacd <= 0.0f && wadb <= 0.0f && uad > 0.0f && vad > 0.0f)
		{
			/* Line region AD. */
			simplex[1] = simplex[3];
			barycentricCoords[0] = uad;
			barycentricCoords[1] = vad;
			dimension = 2;
		}
		else if (ucbd <= 0.0f && uadb <= 0.0f && ubd > 0.0f && ubd > 0.0f)
		{
			/* Line region BD. */
			simplex[0] = simplex[1];
			simplex[1] = simplex[3];
			barycentricCoords[0] = ubd;
			barycentricCoords[1] = vbd;
			dimension = 2;
		}
		else
		{
			/* Calculate the volume of the tetrahedron regions. */
			const float denom = box(db, cb, ab);
			const float volume = denom == 0.0f ? 1.0f : recip(denom);
			const float uabcd = box(simplex[1], simplex[2], simplex[3]) * volume;
			const float vabcd = box(simplex[3], simplex[2], simplex[0]) * volume;
			const float wabcd = box(simplex[1], simplex[3], simplex[0]) * volume;
			const float xabcd = box(simplex[2], simplex[1], simplex[0]) * volume;

			/* The point also wasn't in a line region, so we move on to triangle regions, and the tetrahedron last. */
			if (xabcd < 0.0f && uabc > 0.0f && vabc > 0.0f && wabc > 0.0f)
			{
				/* Face region ABC. */
				barycentricCoords[0] = uabc;
				barycentricCoords[1] = vabc;
				barycentricCoords[2] = wabc;
				dimension = 3;
			}
			else if (uabcd < 0.0f && ucbd > 0.0f && vcbd > 0.0f && wcbd > 0.0f)
			{
				/* Face region CBD. */
				simplex[0] = simplex[2];
				simplex[2] = simplex[3];
				barycentricCoords[0] = ucbd;
				barycentricCoords[1] = vcbd;
				barycentricCoords[2] = wcbd;
				dimension = 3;
			}
			else if (vabcd < 0.0f && uacd > 0.0f && vacd > 0.0f && wacd > 0.0f)
			{
				/* Face region ACD. */
				simplex[1] = simplex[2];
				simplex[2] = simplex[3];
				barycentricCoords[0] = uacd;
				barycentricCoords[1] = vacd;
				barycentricCoords[2] = wacd;
				dimension = 3;
			}
			else if (wabcd < 0.0f && uadb > 0.0f && vadb > 0.0f && wadb > 0.0f)
			{
				/* Face region ADB. */
				simplex[2] = simplex[1];
				simplex[1] = simplex[3];
				barycentricCoords[0] = uadb;
				barycentricCoords[1] = vadb;
				barycentricCoords[2] = wadb;
				dimension = 3;
			}
			else
			{
				/* Region ABCD. */
				barycentricCoords[0] = uabcd;
				barycentricCoords[1] = vabcd;
				barycentricCoords[2] = wabcd;
				barycentricCoords[3] = xabcd;
				dimension = 4;
			}
		}
	}
}