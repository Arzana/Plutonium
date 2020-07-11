#include "Physics/Systems/ContactSolverSystem.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ContactSystem.h"
#include "Physics/Systems/MovementSystem.h"
#include "Core/Diagnostics/Profiler.h"

#define avx_realloc(ptr, type, cnt)	ptr = reinterpret_cast<type*>(_aligned_realloc(ptr, sizeof(type) * cnt, sizeof(type)))

Pu::ContactSolverSystem::ContactSolverSystem(PhysicalWorld & world)
	: world(&world), capacity(0), fcor(nullptr), scor(nullptr), fcof(nullptr),
	scof(nullptr), vx(nullptr), vy(nullptr), vz(nullptr), fimass(nullptr),
	simass(nullptr), jx(nullptr), jy(nullptr), jz(nullptr)
{}

Pu::ContactSolverSystem::ContactSolverSystem(ContactSolverSystem && value)
	: world(value.world), moi(std::move(value.moi)), imass(std::move(value.imass)),
	coefficients(std::move(value.coefficients)), fcor(value.fcor), scor(value.scor),
	fcof(value.fcof), scof(value.scof), vx(value.vx), vy(value.vy), vz(value.vz),
	fimass(value.fimass), simass(value.simass), jx(value.jx), jy(value.jy), 
	jz(value.jz), capacity(value.capacity)
{
	value.capacity = 0;
}

Pu::ContactSolverSystem & Pu::ContactSolverSystem::operator=(ContactSolverSystem && other)
{
	if (this != &other)
	{
		Destroy();

		world = other.world;
		moi = std::move(other.moi);
		imass = std::move(other.imass);
		coefficients = std::move(other.coefficients);
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
		capacity = other.capacity;

		other.capacity = 0;
	}

	return *this;
}

void Pu::ContactSolverSystem::AddItem(PhysicsHandle handle, const Matrix3 & MoI, float mass, float CoR, float CoF)
{
	moi.emplace(handle, MoI);
	imass.emplace(handle, recip(mass));
	coefficients.emplace(handle, Vector2{ CoR, CoF });
}

void Pu::ContactSolverSystem::RemoveItem(PhysicsHandle handle)
{
	moi.erase(handle);
	imass.erase(handle);
	coefficients.erase(handle);
}

void Pu::ContactSolverSystem::SolveConstriants(void)
{
	/* We don't need to solve anything if there are no collisions. */
	if (world->sysCnst->hfirsts.size())
	{
		if constexpr (PhysicsProfileSystems) Profiler::Begin("Solver", Color::SunDawn());

		/* First we fill the temporary SSE buffer with our solver data. */
		EnsureBufferSize();
		FillBuffers();

		/* Then we solve these constraints in SSE and apply the impulses the the movement system. */
		VectorSolve();
		ApplyImpulses();

		if constexpr (PhysicsProfileSystems) Profiler::End();
	}
}

void Pu::ContactSolverSystem::EnsureBufferSize(void)
{
	/* The temporary buffers might need to be resized if we have more collisions than we can currently handle. */
	const size_t count = world->sysCnst->hfirsts.size();
	if (capacity < count)
	{
		/* We can maximally apply 2 impulses per collisions (one for each object). */
		capacity = count;
		const size_t impulseCount = count << 1;

		avx_realloc(fcor, ofloat, count);
		avx_realloc(scor, ofloat, count);
		avx_realloc(fcof, ofloat, count);
		avx_realloc(scof, ofloat, count);
		avx_realloc(vx, ofloat, count);
		avx_realloc(vy, ofloat, count);
		avx_realloc(vz, ofloat, count);
		avx_realloc(fimass, ofloat, count);
		avx_realloc(simass, ofloat, count);
		avx_realloc(jx, ofloat, impulseCount);
		avx_realloc(jy, ofloat, impulseCount);
		avx_realloc(jz, ofloat, impulseCount);
	}
}

/*
MSVC thinks that the temporary values can be used before they are being set.
This is not the case, we check against i to see whether they have been set.
*/
#pragma warning(push)
#pragma warning(disable:4701)
void Pu::ContactSolverSystem::FillBuffers(void)
{
	/* Use these buffers as staging buffer for the AVX types. */
	AVX_FLOAT_UNION tmp_fcor;
	AVX_FLOAT_UNION tmp_scor;
	AVX_FLOAT_UNION tmp_fcof;
	AVX_FLOAT_UNION tmp_scof;
	AVX_FLOAT_UNION tmp_vx;
	AVX_FLOAT_UNION tmp_vy;
	AVX_FLOAT_UNION tmp_vz;
	AVX_FLOAT_UNION tmp_fimass;
	AVX_FLOAT_UNION tmp_simass;

	const size_t size = world->sysCnst->hfirsts.size();
	size_t i;

	/* Loop through all the registered collisions. */
	for (i = 0; i < size; i++)
	{
		/* Get the handles of the current collision. */
		const PhysicsHandle hfirst = world->sysCnst->hfirsts[i];
		const PhysicsHandle hsecond = world->sysCnst->hseconds[i];

		/* Gets the AVX index (j) and the packed index (k). */
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		/* We have to fill the buffers with different data depending on whether one of the types was static. */
		const bool isStatic = physics_get_type(hfirst) == PhysicsType::Static;

		/* 
		The relative velocity is either the velocity of the second object (incase of a static collision) or the delta velocity between the two kinematic objects.
		The mass scalar also needs to be set differently, we don't care about mass in static collisions as all energy will stay in the kinematic object.
		*/
		Vector3 v;
		if (isStatic)
		{
			v = world->sysMove->GetVelocity(world->QueryInternalIndex(hsecond));
			tmp_fimass.V[k] = 0.0f;
			tmp_simass.V[k] = 1.0f;
		}
		else
		{
			const uint16 idx1 = world->QueryInternalIndex(hfirst);
			const uint16 idx2 = world->QueryInternalIndex(hsecond);
			v = world->sysMove->GetVelocity(idx2) - world->sysMove->GetVelocity(idx1);

			tmp_fimass.V[k] = imass[hfirst];
			tmp_simass.V[k] = imass[hsecond];
		}

		/* Gets the coefficients of both objects. */
		const Vector2 coeff1 = coefficients[hfirst];
		const Vector2 coeff2 = coefficients[hsecond];

		tmp_fcor.V[k] = coeff1.X;
		tmp_scor.V[k] = coeff2.X;
		tmp_fcof.V[k] = coeff1.Y;
		tmp_scof.V[k] = coeff2.Y;
		tmp_vx.V[k] = v.X;
		tmp_vy.V[k] = v.Y;
		tmp_vz.V[k] = v.Z;

		/* Push the staging buffer to the output. */
		if (k >= 7 || i == size - 1)
		{
			fcor[j] = tmp_fcor.SIMD;
			scor[j] = tmp_scor.SIMD;
			fcof[j] = tmp_fcof.SIMD;
			scof[j] = tmp_scof.SIMD;
			vx[j] = tmp_vx.SIMD;
			vy[j] = tmp_vy.SIMD;
			vz[j] = tmp_vz.SIMD;
			fimass[j] = tmp_fimass.SIMD;
			simass[j] = tmp_simass.SIMD;
		}
	}
}
#pragma warning(pop)

/*
	Linear impulse:
	M = m1^-1 + m2^-1
	e = min(coefficient of resitution)
	j = -(1.0f + e) * dot(V, N) / M
	V1 -= j * N * m1^-1
	V2 += j * N * m2^-1

	Friction (broken):
	T = normalize(V - dot(V, N) * N)
	e = sqrt(product coefficient of friction)
	j = clamp((-(1.0f + e) * dot(V, T)) / M, -j * e, j * e)
	V1 -= j * T * m1^-1
	V2 += j * T * m2^-1
*/
void Pu::ContactSolverSystem::VectorSolve(void)
{
	/* Predefine often used constants. */
	const size_t count = world->sysCnst->hfirsts.size();
	const size_t avxCnt = (count >> 0x3) + (count != 0);
	const ofloat zero = _mm256_setzero_ps();
	const ofloat one = _mm256_set1_ps(1.0f);
	const ofloat neg = _mm256_set1_ps(-1.0f);

	/* Cache pointers to the collision normal. */
	const ofloat *nx = world->sysCnst->nx.data();
	const ofloat *ny = world->sysCnst->ny.data();
	const ofloat *nz = world->sysCnst->nz.data();

	/* Solve collision per AVX type. */
	for (size_t i = 0, k = avxCnt; i < avxCnt; i++, k++)
	{
		/* Calculate linear impulse. */
		const ofloat imassTotal = _mm256_rcp_ps(_mm256_add_ps(fimass[i], simass[i]));
		ofloat cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], nx[i]), _mm256_add_ps(_mm256_mul_ps(vy[i], ny[i]), _mm256_mul_ps(vz[i], nz[i])));
		ofloat e = _mm256_mul_ps(_mm256_add_ps(_mm256_min_ps(fcor[i], scor[i]), one), neg);
		ofloat j = _mm256_mul_ps(_mm256_mul_ps(e, cosTheta), imassTotal);

		/* Apply linear impulse to the first object. */
		jx[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, nx[i]), fimass[i]), neg);
		jy[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, ny[i]), fimass[i]), neg);
		jz[k] = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(j, nz[i]), fimass[i]), neg);

		/* Apply linear impulse to the second object. */
		jx[i] = _mm256_mul_ps(_mm256_mul_ps(j, nx[i]), simass[i]);
		jy[i] = _mm256_mul_ps(_mm256_mul_ps(j, ny[i]), simass[i]);
		jz[i] = _mm256_mul_ps(_mm256_mul_ps(j, nz[i]), simass[i]);

		/* Calculate tangent vector (normalization is done safely incase of N=V). */
		ofloat tx = _mm256_sub_ps(vx[i], _mm256_mul_ps(cosTheta, nx[i]));
		ofloat ty = _mm256_sub_ps(vy[i], _mm256_mul_ps(cosTheta, ny[i]));
		ofloat tz = _mm256_sub_ps(vz[i], _mm256_mul_ps(cosTheta, nz[i]));
		ofloat l = _mm256_add_ps(_mm256_mul_ps(tx, tx), _mm256_add_ps(_mm256_mul_ps(ty, ty), _mm256_mul_ps(tz, tz)));
		l = _mm256_andnot_ps(_mm256_cmp_ps(zero, l, _CMP_EQ_OQ), _mm256_rsqrt_ps(l));
		tx = _mm256_mul_ps(tx, l);
		ty = _mm256_mul_ps(ty, l);
		tz = _mm256_mul_ps(tz, l);

		/* Calculate friction impulse. */
		cosTheta = _mm256_add_ps(_mm256_mul_ps(vx[i], tx), _mm256_add_ps(_mm256_mul_ps(vy[i], ty), _mm256_mul_ps(vz[i], tz)));
		e = _mm256_sqrt_ps(_mm256_mul_ps(fcof[i], scof[i]));
		j = _mm256_max_ps(_mm256_mul_ps(_mm256_mul_ps(j, neg), e), _mm256_min_ps(_mm256_mul_ps(j, e), _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(_mm256_add_ps(e, one), neg), cosTheta), imassTotal)));

		/* Apply friction impulse to the first object. */
		jx[k] = _mm256_sub_ps(jx[k], _mm256_mul_ps(_mm256_mul_ps(j, tx), fimass[i]));
		jy[k] = _mm256_sub_ps(jy[k], _mm256_mul_ps(_mm256_mul_ps(j, ty), fimass[i]));
		jz[k] = _mm256_sub_ps(jz[k], _mm256_mul_ps(_mm256_mul_ps(j, tz), fimass[i]));

		/* Apply friction impulse to the second object. */
		jx[i] = _mm256_add_ps(jx[i], _mm256_mul_ps(_mm256_mul_ps(j, tx), simass[i]));
		jy[i] = _mm256_add_ps(jy[i], _mm256_mul_ps(_mm256_mul_ps(j, ty), simass[i]));
		jz[i] = _mm256_add_ps(jz[i], _mm256_mul_ps(_mm256_mul_ps(j, tz), simass[i]));
	}
}

void Pu::ContactSolverSystem::ApplyImpulses(void)
{
	const size_t count = world->sysCnst->hfirsts.size();
	const size_t avxCnt = (count >> 0x3) + (count != 0);

	/* Push the accumulated forces to the movement system. */
	for (size_t i = 0; i < count; i++)
	{
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;
		
		const PhysicsHandle hfirst = world->sysCnst->hfirsts[i];
		const PhysicsHandle hsecond = world->sysCnst->hseconds[i];

		/* The second object will always have impulses applied to it as it's either kinematic or dynamic. */
		float x = AVX_FLOAT_UNION{ jx[j] }.V[k];
		float y = AVX_FLOAT_UNION{ jy[j] }.V[k];
		float z = AVX_FLOAT_UNION{ jz[j] }.V[k];
		world->sysMove->AddForce(world->QueryInternalIndex(hsecond), x, y, z, Quaternion{});

		/* The second object might not need impulses to be applied. */
		if (physics_get_type(hfirst) != PhysicsType::Static)
		{
			x = AVX_FLOAT_UNION{ jx[j + avxCnt] }.V[k];
			y = AVX_FLOAT_UNION{ jy[j + avxCnt] }.V[k];
			z = AVX_FLOAT_UNION{ jz[j + avxCnt] }.V[k];
			world->sysMove->AddForce(world->QueryInternalIndex(hfirst), x, y, z, Quaternion{});
		}
	}
}

void Pu::ContactSolverSystem::Destroy(void)
{
	if (capacity)
	{
		_aligned_free(fcor);
		_aligned_free(scor);
		_aligned_free(fcof);
		_aligned_free(scof);
		_aligned_free(vx);
		_aligned_free(vy);
		_aligned_free(vz);
		_aligned_free(fimass);
		_aligned_free(simass);
		_aligned_free(jx);
		_aligned_free(jy);
		_aligned_free(jz);
	}
}