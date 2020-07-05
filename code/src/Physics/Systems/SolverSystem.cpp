#include "Physics/Systems/SolverSystem.h"
#include "Physics/Systems/PhysicalWorld.h"

#define avx_realloc(ptr, type, cnt)	ptr = reinterpret_cast<type*>(_aligned_realloc(ptr, sizeof(type) * cnt, sizeof(type)))

static Pu::uint32 collisions = 0;

Pu::SolverSystem::SolverSystem(PhysicalWorld & world)
	: world(&world), sharedCapacity(0), kinematicCapacity(0), resultCapacity(0)
{}

Pu::SolverSystem::SolverSystem(SolverSystem && value)
	: world(value.world), moi(std::move(value.moi)), imass(std::move(value.imass)),
	coefficients(std::move(value.coefficients)), manifolds(std::move(value.manifolds)),
	nx(value.nx), ny(value.ny), nz(value.nz), hfirst(value.hfirst), hsecond(value.hsecond),
	fcor(value.fcor), scor(value.scor), fcof(value.fcof), scof(value.scof), vx(value.vx), vy(value.vy),
	vz(value.vz), fimass(value.fimass), simass(value.simass), jx(value.jx), jy(value.jy), jz(value.jz),
	sharedCapacity(value.sharedCapacity), kinematicCapacity(value.kinematicCapacity)
{
	value.sharedCapacity = 0;
	value.kinematicCapacity = 0;
}

Pu::SolverSystem & Pu::SolverSystem::operator=(SolverSystem && other)
{
	if (this != &other)
	{
		Destroy();

		world = other.world;
		moi = std::move(other.moi);
		imass = std::move(other.imass);
		coefficients = std::move(other.coefficients);
		manifolds = std::move(other.manifolds);
		nx = other.nx;
		ny = other.ny;
		nz = other.nz;
		hfirst = other.hfirst;
		hsecond = other.hsecond;
		fcor = other.fcor;
		scor = other.scor;
		fcof = other.fcof;
		scof = other.scof;
		vx = other.vx;
		vy = other.vy;
		vz = other.vz;
		fimass = other.fimass;
		simass = other.simass;
		jx = other.jx;
		jy = other.jy;
		jz = other.jz;
		sharedCapacity = other.sharedCapacity;
		kinematicCapacity = other.kinematicCapacity;

		other.sharedCapacity = 0;
		other.kinematicCapacity = 0;
	}

	return *this;
}

Pu::uint32 Pu::SolverSystem::GetCollisionCount(void)
{
	return collisions;
}

void Pu::SolverSystem::ResetCounter(void)
{
	collisions = 0;
}

void Pu::SolverSystem::AddItem(PhysicsHandle handle, const Matrix3 & MoI, float mass, float CoR, float CoF)
{
	moi.emplace(handle, MoI);
	imass.emplace(handle, recip(mass));
	coefficients.emplace(handle, Vector2{ CoR, CoF });
}

void Pu::SolverSystem::RemoveItem(PhysicsHandle handle)
{
	moi.erase(handle);
	imass.erase(handle);
	coefficients.erase(handle);
}

void Pu::SolverSystem::RegisterCollision(const CollisionManifold & manifold)
{
	manifolds.emplace_back(manifold);
}

void Pu::SolverSystem::SolveConstriants(void)
{
	size_t cntStatic = 0, cntKinematic = 0;
	EnsureBufferSize(cntStatic, cntKinematic);

	if (cntStatic)
	{
		FillStatic(cntStatic);
		SolveStatic(cntStatic);
	}

	if (cntKinematic)
	{
		FillKinematic(cntKinematic);
		SolveKinematic(cntKinematic);
	}

	collisions += static_cast<uint32>(cntStatic + cntKinematic);
	manifolds.clear();
}

void Pu::SolverSystem::EnsureBufferSize(size_t & staticCount, size_t & kinematicCount)
{
	/* Pre-calculate the buffer size to save on malloc calls. */
	for (const CollisionManifold &cur : manifolds)
	{
		const bool isStatic = physics_get_type(cur.FirstObject) == PhysicsType::Static;
		staticCount += isStatic;
		kinematicCount += !isStatic;
	}

	/*
	Kinematic collisions need more information than static collisions.
	Kinematic collisions also have two result (one for each object), so the result buffer is bigger.
	*/
	const size_t bufferSize = max(staticCount, kinematicCount);
	const size_t resultSize = max(staticCount, kinematicCount << 1);
	if (sharedCapacity < bufferSize)
	{
		sharedCapacity = bufferSize;

		avx_realloc(nx, ofloat, bufferSize);
		avx_realloc(ny, ofloat, bufferSize);
		avx_realloc(nz, ofloat, bufferSize);
		avx_realloc(hsecond, int256, bufferSize);
		avx_realloc(fcor, ofloat, bufferSize);
		avx_realloc(scor, ofloat, bufferSize);
		avx_realloc(fcof, ofloat, bufferSize);
		avx_realloc(scof, ofloat, bufferSize);
		avx_realloc(vx, ofloat, bufferSize);
		avx_realloc(vy, ofloat, bufferSize);
		avx_realloc(vz, ofloat, bufferSize);
	}

	if (kinematicCapacity < kinematicCount)
	{
		kinematicCapacity = kinematicCount;

		avx_realloc(hfirst, int256, kinematicCount);
		avx_realloc(fimass, ofloat, kinematicCount);
		avx_realloc(simass, ofloat, kinematicCount);
	}

	if (resultCapacity < resultSize)
	{
		resultCapacity = resultSize;

		avx_realloc(jx, ofloat, resultSize);
		avx_realloc(jy, ofloat, resultSize);
		avx_realloc(jz, ofloat, resultSize);
	}
}

void Pu::SolverSystem::FillStatic(size_t staticCount)
{
	/* Use these buffers as staging buffer for the AVX types. */
	AVX_FLOAT_UNION tmp_nx;
	AVX_FLOAT_UNION tmp_ny;
	AVX_FLOAT_UNION tmp_nz;
	AVX_UINT_UNION tmp_hsecond;
	AVX_FLOAT_UNION tmp_fcor;
	AVX_FLOAT_UNION tmp_scor;
	AVX_FLOAT_UNION tmp_fcof;
	AVX_FLOAT_UNION tmp_scof;
	AVX_FLOAT_UNION tmp_vx;
	AVX_FLOAT_UNION tmp_vy;
	AVX_FLOAT_UNION tmp_vz;

	/* Fill the calculation buffer with the static manifolds. */
	size_t i = 0;
	for (const CollisionManifold &cur : manifolds)
	{
		if (physics_get_type(cur.FirstObject) == PhysicsType::Static)
		{
			/* Calculate the result index (j), AVX index [0, 7] (k), and the internal index (idx). */
			const size_t j = i >> 0x3;
			const size_t k = i++ & 0x7;
			const uint16 idx = world->QueryInternalIndex(cur.SecondObject);

			/* Query the various systems for additional parameters. */
			const Vector2 cofs1 = coefficients[cur.FirstObject];
			const Vector2 cofs2 = coefficients[cur.SecondObject];
			const Vector3 vloc = world->sysMove->GetVelocity(idx);

			/* Fill the next spot in the AVX staging buffer. */
			tmp_nx.V[k] = cur.N.X;
			tmp_ny.V[k] = cur.N.Y;
			tmp_nz.V[k] = cur.N.Z;
			tmp_hsecond.V[k] = idx;
			tmp_fcor.V[k] = cofs1.X;
			tmp_scor.V[k] = cofs2.X;
			tmp_fcof.V[k] = cofs1.Y;
			tmp_scof.V[k] = cofs2.Y;
			tmp_vx.V[k] = vloc.X;
			tmp_vy.V[k] = vloc.Y;
			tmp_vz.V[k] = vloc.Z;

			/* Push the staging buffer to the output. */
			if (k >= 7 || i == staticCount)
			{
				nx[j] = tmp_nx.AVX;
				ny[j] = tmp_ny.AVX;
				nz[j] = tmp_nz.AVX;
				hsecond[j] = tmp_hsecond.AVX;
				fcor[j] = tmp_fcor.AVX;
				scor[j] = tmp_scor.AVX;
				fcof[j] = tmp_fcof.AVX;
				scof[j] = tmp_scof.AVX;
				vx[j] = tmp_vx.AVX;
				vy[j] = tmp_vy.AVX;
				vz[j] = tmp_vz.AVX;

				/* We've found all the static objects, early out. */
				if (i == staticCount) return;
			}
		}
	}
}

void Pu::SolverSystem::FillKinematic(size_t & kinematicCount)
{
	/* Use these buffers as staging buffer for the AVX types. */
	AVX_FLOAT_UNION tmp_nx;
	AVX_FLOAT_UNION tmp_ny;
	AVX_FLOAT_UNION tmp_nz;
	AVX_UINT_UNION tmp_hfirst;
	AVX_UINT_UNION tmp_hsecond;
	AVX_FLOAT_UNION tmp_fcor;
	AVX_FLOAT_UNION tmp_scor;
	AVX_FLOAT_UNION tmp_fcof;
	AVX_FLOAT_UNION tmp_scof;
	AVX_FLOAT_UNION tmp_vx;
	AVX_FLOAT_UNION tmp_vy;
	AVX_FLOAT_UNION tmp_vz;
	AVX_FLOAT_UNION tmp_fimass;
	AVX_FLOAT_UNION tmp_simass;

	size_t i = 0;
	for (const CollisionManifold &cur : manifolds)
	{
		if (physics_get_type(cur.FirstObject) == PhysicsType::Kinematic)
		{
			/*
			Calculate
			- The result index (j)
			- The AVX index [0, 7] (k)
			- The first internal index (l1)
			- The second internal index (l2)
			*/
			const size_t j = i >> 0x3;
			const size_t k = i & 0x7;
			const uint16 l1 = world->QueryInternalIndex(cur.FirstObject);
			const uint16 l2 = world->QueryInternalIndex(cur.SecondObject);

			/*
			Query the various systems for additional parameters.
			Also ignore this collision if the objects are already moving away from each other,
			in the direction of the collision normal.
			*/
			const Vector3 relVloc = world->sysMove->GetVelocity(l2) - world->sysMove->GetVelocity(l1);
			if (dot(relVloc, cur.N) > 0.0f) continue;

			++i;
			const Vector2 cofs1 = coefficients[cur.FirstObject];
			const Vector2 cofs2 = coefficients[cur.SecondObject];

			/* Fill the next spot in the AVX staging buffer. */
			tmp_nx.V[k] = cur.N.X;
			tmp_ny.V[k] = cur.N.Y;
			tmp_nz.V[k] = cur.N.Z;
			tmp_hfirst.V[k] = l1;
			tmp_hsecond.V[k] = l2;
			tmp_fcor.V[k] = cofs1.X;
			tmp_scor.V[k] = cofs2.X;
			tmp_fcof.V[k] = cofs1.Y;
			tmp_scof.V[k] = cofs2.Y;
			tmp_vx.V[k] = relVloc.X;
			tmp_vy.V[k] = relVloc.Y;
			tmp_vz.V[k] = relVloc.Z;
			tmp_fimass.V[k] = imass[cur.FirstObject];
			tmp_simass.V[k] = imass[cur.SecondObject];

			/* Push the staging buffer to the output. */
			if (k >= 7)
			{
				nx[j] = tmp_nx.AVX;
				ny[j] = tmp_ny.AVX;
				nz[j] = tmp_nz.AVX;
				hfirst[j] = tmp_hfirst.AVX;
				hsecond[j] = tmp_hsecond.AVX;
				fcor[j] = tmp_fcor.AVX;
				scor[j] = tmp_scor.AVX;
				fcof[j] = tmp_fcof.AVX;
				scof[j] = tmp_scof.AVX;
				vx[j] = tmp_vx.AVX;
				vy[j] = tmp_vy.AVX;
				vz[j] = tmp_vz.AVX;
				fimass[j] = tmp_fimass.AVX;
				simass[j] = tmp_simass.AVX;
			}
		}
	}

	/* Push the staging buffer one more time if needed. */
	if (i && (i & 0x7) < 7)
	{
		const size_t j = i >> 0x3;

		nx[j] = tmp_nx.AVX;
		ny[j] = tmp_ny.AVX;
		nz[j] = tmp_nz.AVX;
		hfirst[j] = tmp_hfirst.AVX;
		hsecond[j] = tmp_hsecond.AVX;
		fcor[j] = tmp_fcor.AVX;
		scor[j] = tmp_scor.AVX;
		fcof[j] = tmp_fcof.AVX;
		scof[j] = tmp_scof.AVX;
		vx[j] = tmp_vx.AVX;
		vy[j] = tmp_vy.AVX;
		vz[j] = tmp_vz.AVX;
		fimass[j] = tmp_fimass.AVX;
		simass[j] = tmp_simass.AVX;
	}

	/* Update the kinematic count to the actual objects in the buffer. */
	kinematicCount = i;
}

/*
	Linear impulse:
	e = min(coefficient of resitution)
	j = max(0, -(1.0f + e) * dot(V, N))
	V += j * N

	Friction:
	T = normalize(V - dot(V, N) * N)
	e = sqrt(product coefficient of friction)
	j = clamp(-(1.0f + e) * dot(V, T), -j * e, j * e)
	V += j * T;
*/
void Pu::SolverSystem::SolveStatic(size_t count)
{
	/* Predefine often used constants. */
	const size_t avxCnt = (count >> 0x3) + (count != 0);
	const ofloat zero = _mm256_setzero_ps();
	const ofloat one = _mm256_set1_ps(1.0f);
	const ofloat neg = _mm256_set1_ps(-1.0f);

	/* Solve collision per AVX type. */
	for (size_t i = 0; i < avxCnt; i++)
	{
		/* Calculate linear impulse. */
		ofloat cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], nx[i]), _mm256_add_ps(_mm256_mul_ps(vy[i], ny[i]), _mm256_mul_ps(vz[i], nz[i])));
		ofloat e = _mm256_mul_ps(_mm256_add_ps(_mm256_min_ps(fcor[i], scor[i]), one), neg);
		ofloat j = _mm256_max_ps(zero, _mm256_mul_ps(e, cosTheta));

		/* Apply linear impulse. */
		jx[i] = _mm256_mul_ps(j, nx[i]);
		jy[i] = _mm256_mul_ps(j, ny[i]);
		jz[i] = _mm256_mul_ps(j, nz[i]);

		/* Calculate tangent vector. */
		ofloat tx = _mm256_sub_ps(vx[i], _mm256_mul_ps(cosTheta, nx[i]));
		ofloat ty = _mm256_sub_ps(vy[i], _mm256_mul_ps(cosTheta, ny[i]));
		ofloat tz = _mm256_sub_ps(vz[i], _mm256_mul_ps(cosTheta, nz[i]));
		const ofloat l = _mm256_rsqrt_ps(_mm256_add_ps(_mm256_mul_ps(tx, tx), _mm256_add_ps(_mm256_mul_ps(ty, ty), _mm256_mul_ps(tz, tz))));
		tx = _mm256_mul_ps(tx, l);
		ty = _mm256_mul_ps(ty, l);
		tz = _mm256_mul_ps(tz, l);

		/* Calculate friction impulse. */
		cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], tx), _mm256_add_ps(_mm256_mul_ps(vy[i], ty), _mm256_mul_ps(vz[i], tz)));
		e = _mm256_sqrt_ps(_mm256_mul_ps(fcof[i], scof[i]));
		j = _mm256_max_ps(_mm256_mul_ps(_mm256_mul_ps(j, neg), e), _mm256_min_ps(_mm256_mul_ps(j, e), _mm256_mul_ps(_mm256_mul_ps(_mm256_add_ps(one, e), neg), cosTheta)));

		/* Apply friction impulse. */
		jx[i] = _mm256_add_ps(jx[i], _mm256_mul_ps(j, tx));
		jy[i] = _mm256_add_ps(jy[i], _mm256_mul_ps(j, ty));
		jz[i] = _mm256_add_ps(jz[i], _mm256_mul_ps(j, tz));
	}

	/* Push the accumulated forces to the movement system. */
	for (size_t i = 0; i < count; i++)
	{
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		const uint32 id = AVX_UINT_UNION{ hsecond[j] }.V[k];
		const float x = AVX_FLOAT_UNION{ jx[j] }.V[k];
		const float y = AVX_FLOAT_UNION{ jy[j] }.V[k];
		const float z = AVX_FLOAT_UNION{ jz[j] }.V[k];

		world->sysMove->AddForce(id, x, y, z, Quaternion{});
	}
}

/*
	Linear impulse:
	M = m1^-1 + m2^-1
	e = min(coefficient of resitution)
	j = -(1.0f + e) * dot(V, N) / M
	V1 -= j * N * m1^-1
	V2 += j * N * m2^-1

	Friction:
	T = normalize(V - dot(V, N) * N)
	e = sqrt(product coefficient of friction)
	j = clamp((-(1.0f + e) * dot(V, T)) / M, -j * e, j * e)
	V1 -= j * T * m1^-1
	V2 += j * T * m2^-1
*/
void Pu::SolverSystem::SolveKinematic(size_t count)
{
	/* Predefine often used constants. */
	const size_t avxCnt = (count >> 0x3) + (count != 0);
	const ofloat zero = _mm256_setzero_ps();
	const ofloat one = _mm256_set1_ps(1.0f);
	const ofloat neg = _mm256_set1_ps(-1.0f);

	/* Solve collision per AVX type. */
	for (size_t i = 0, k = avxCnt; i < avxCnt; i++, k++)
	{
		/* Calculate linear impulse. */
		const ofloat imassTotal = _mm256_rcp_ps(_mm256_add_ps(fimass[i], simass[i]));
		ofloat cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], nx[i]), _mm256_add_ps(_mm256_mul_ps(vy[i], ny[i]), _mm256_mul_ps(vz[i], nz[i])));
		ofloat e = _mm256_mul_ps(_mm256_add_ps(_mm256_min_ps(fcor[i], scor[i]), one), neg);
		ofloat j = _mm256_mul_ps(_mm256_mul_ps(e, cosTheta), imassTotal);

		/* Apply linear impulse to the first object. */
		jx[i] = _mm256_mul_ps(_mm256_mul_ps(j, nx[i]), fimass[i]);
		jy[i] = _mm256_mul_ps(_mm256_mul_ps(j, ny[i]), fimass[i]);
		jz[i] = _mm256_mul_ps(_mm256_mul_ps(j, nz[i]), fimass[i]);

		/* Apply linear impulse to the second object. */
		jx[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, nx[i]), simass[i]), neg);
		jy[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, ny[i]), simass[i]), neg);
		jz[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, nz[i]), simass[i]), neg);

		/* Calculate tangent vector. */
		ofloat tx = _mm256_sub_ps(vx[i], _mm256_mul_ps(cosTheta, nx[i]));
		ofloat ty = _mm256_sub_ps(vy[i], _mm256_mul_ps(cosTheta, ny[i]));
		ofloat tz = _mm256_sub_ps(vz[i], _mm256_mul_ps(cosTheta, nz[i]));
		const ofloat l = _mm256_rsqrt_ps(_mm256_add_ps(_mm256_mul_ps(tx, tx), _mm256_add_ps(_mm256_mul_ps(ty, ty), _mm256_mul_ps(tz, tz))));
		tx = _mm256_mul_ps(tx, l);
		ty = _mm256_mul_ps(ty, l);
		tz = _mm256_mul_ps(tz, l);

		/* Calculate friction impulse. */
		cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], tx), _mm256_add_ps(_mm256_mul_ps(vy[i], ty), _mm256_mul_ps(vz[i], tz)));
		e = _mm256_sqrt_ps(_mm256_mul_ps(fcof[i], scof[i]));
		j = _mm256_max_ps(_mm256_mul_ps(_mm256_mul_ps(j, neg), e), _mm256_min_ps(_mm256_mul_ps(j, e), _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(_mm256_add_ps(e, one), neg), cosTheta), imassTotal)));

		/* Apply friction impulse to the first object. */
		jx[i] = _mm256_add_ps(jx[i], _mm256_mul_ps(_mm256_mul_ps(j, tx), fimass[i]));
		jy[i] = _mm256_add_ps(jy[i], _mm256_mul_ps(_mm256_mul_ps(j, ty), fimass[i]));
		jz[i] = _mm256_add_ps(jz[i], _mm256_mul_ps(_mm256_mul_ps(j, tz), fimass[i]));

		/* Apply friction impulse to the second object. */
		jx[k] = _mm256_sub_ps(jx[k], _mm256_mul_ps(_mm256_mul_ps(j, tx), simass[i]));
		jy[k] = _mm256_sub_ps(jy[k], _mm256_mul_ps(_mm256_mul_ps(j, ty), simass[i]));
		jz[k] = _mm256_sub_ps(jz[k], _mm256_mul_ps(_mm256_mul_ps(j, tz), simass[i]));
	}

	/* Push the accumulated forces to the movement system. */
	for (size_t i = 0; i < count; i++)
	{
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		uint32 id = AVX_UINT_UNION{ hfirst[j] }.V[k];
		float x = AVX_FLOAT_UNION{ jx[j] }.V[k];
		float y = AVX_FLOAT_UNION{ jy[j] }.V[k];
		float z = AVX_FLOAT_UNION{ jz[j] }.V[k];
		world->sysMove->AddForce(id, x, y, z, Quaternion{});

		id = AVX_UINT_UNION{ hsecond[j] }.V[k];
		x = AVX_FLOAT_UNION{ jx[j + avxCnt] }.V[k];
		y = AVX_FLOAT_UNION{ jy[j + avxCnt] }.V[k];
		z = AVX_FLOAT_UNION{ jz[j + avxCnt] }.V[k];
		world->sysMove->AddForce(id, x, y, z, Quaternion{});
	}
}

void Pu::SolverSystem::Destroy(void)
{
	if (sharedCapacity)
	{
		_aligned_free(nx);
		_aligned_free(ny);
		_aligned_free(nz);
		_aligned_free(hsecond);
		_aligned_free(fcor);
		_aligned_free(scor);
		_aligned_free(fcof);
		_aligned_free(scof);
		_aligned_free(vx);
		_aligned_free(vy);
		_aligned_free(vz);
	}

	if (kinematicCapacity)
	{
		_aligned_free(hfirst);
		_aligned_free(fimass);
		_aligned_free(simass);
	}
	if (resultCapacity)
	{
		_aligned_free(jx);
		_aligned_free(jy);
		_aligned_free(jz);
	}
}