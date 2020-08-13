#include "Physics/Systems/ContactSolverSystem.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Physics/Systems/ContactSystem.h"
#include "Physics/Systems/MovementSystem.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/Vector3_SIMD.h"
#include "Core/Math/Matrix3_SIMD.h"

#ifdef _DEBUG
#include "Graphics/Diagnostics/DebugRenderer.h"
#define DBG_ADD_FORCE()		appliedForces.emplace_back(TimedForce{ pu_now(), mag, at, f / mag })
#endif

Pu::ContactSolverSystem::ContactSolverSystem(PhysicalWorld & world)
	: world(&world), capacity(0)
{}

/* imass hides class member. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::ContactSolverSystem::AddItem(PhysicsHandle handle, const Matrix3 & iMoI, float imass, const MechanicalProperties & props)
{
	imoi.emplace(handle, iMoI);
	this->imass.emplace(handle, imass);
	coefficients.emplace(handle, props);
}
#pragma warning(pop)

void Pu::ContactSolverSystem::RemoveItem(PhysicsHandle handle)
{
	imoi.erase(handle);
	imass.erase(handle);
	coefficients.erase(handle);
}

void Pu::ContactSolverSystem::SolveConstriants(ofloat dt)
{
	/* We don't need to solve anything if there are no collisions. */
	if (world->sysCnst->hfirsts.size())
	{
		if constexpr (ProfileWorldSystems) Profiler::Begin("Solver", Color::SunDawn());

		/* First we fill the temporary SSE buffer with our solver data. */
		EnsureBufferSize();
		FillBuffers();

		/* Then we solve these constraints in SSE and apply the impulses the the movement system. */
		VectorSolve(dt);
		ApplyImpulses();

		if constexpr (ProfileWorldSystems) Profiler::End();
	}
}

#ifdef _DEBUG
void Pu::ContactSolverSystem::Visualize(DebugRenderer & dbgRenderer) const
{
	/* Remove all the old forces. */
	for (size_t i = 0; i < appliedForces.size();)
	{
		const int64 ms = pu_ms(appliedForces[i].Time, pu_now());
		if ((ms * 0.001f) > PhysicsDebuggingTTL) appliedForces.removeAt(i);
		else ++i;
	}

	/* Display the forces. */
	for (const TimedForce &f : appliedForces)
	{
		dbgRenderer.AddArrow(f.At, f.Dir, Color::WhiteSmoke(), f.Magnitude);
	}
}
#endif

void Pu::ContactSolverSystem::EnsureBufferSize(void)
{
	/* The temporary buffers might need to be resized if we have more collisions than we can currently handle. */
	const size_t count = world->sysCnst->hfirsts.size();
	if (capacity < count)
	{
		/* We can maximally apply 2 impulses per collisions (one for each object). */
		capacity = count;
		const size_t impulseCount = count << 1;

		/* Reallocate the coefficients. */
		cor1 = _mm256_realloc_ps(cor1, count);
		cor2 = _mm256_realloc_ps(cor2, count);
		cof1 = _mm256_realloc_ps(cof1, count);
		cof2 = _mm256_realloc_ps(cof2, count);

		/* Reallocate the positions. */
		_mm256_realloc_v3(px1, py1, pz1, count);
		_mm256_realloc_v3(px2, py2, pz2, count);

		/* Reallocate the velocities. */
		_mm256_realloc_v3(vx1, vy1, vz1, count);
		_mm256_realloc_v3(vx2, vy2, vz2, count);

		/* Reallocate the angular velocities. */
		_mm256_realloc_v3(wp1, wy1, wr1, count);
		_mm256_realloc_v3(wp2, wy2, wr2, count);

		/* Reallocate the inverse masses. */
		imass1 = _mm256_realloc_ps(imass1, count);
		imass2 = _mm256_realloc_ps(imass2, count);

		/* Reallocate the inverse moment of inertia tensors. */
		_mm256_realloc_m3(m001, m011, m021, m101, m111, m121, m201, m211, m221, count);
		_mm256_realloc_m3(m002, m012, m022, m102, m112, m122, m202, m212, m222, count);

		/* Reallocate the result impulses. */
		_mm256_recalloc_v3(jx, jy, jz, impulseCount);
		_mm256_recalloc_v3(jpitch, jyaw, jroll, impulseCount);
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
	AVX_FLOAT_UNION tmp_cor1;
	AVX_FLOAT_UNION tmp_cor2;
	AVX_FLOAT_UNION tmp_cof1;
	AVX_FLOAT_UNION tmp_cof2;
	AVX_VEC3_UNION tmp_p1;
	AVX_VEC3_UNION tmp_p2;
	AVX_VEC3_UNION tmp_v1;
	AVX_VEC3_UNION tmp_v2;
	AVX_VEC3_UNION tmp_w1;
	AVX_VEC3_UNION tmp_w2;
	AVX_FLOAT_UNION tmp_imass1;
	AVX_FLOAT_UNION tmp_imass2;
	AVX_MAT3_UNION tmp_moi1;
	AVX_MAT3_UNION tmp_moi2;

	const size_t size = world->sysCnst->hfirsts.size();
	size_t i;

	/* Loop through all the registered collisions. */
	for (i = 0; i < size; i++)
	{
		/* Get the public handles of the current collision. */
		const PhysicsHandle hfirst = world->sysCnst->hfirsts[i];
		const PhysicsHandle hsecond = world->sysCnst->hseconds[i];

		/* Gets the AVX index (j) and the packed index (k). */
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		/* Gets the coefficients of both objects. */
		const MechanicalProperties &mat1 = coefficients[hfirst];
		const MechanicalProperties &mat2 = coefficients[hsecond];

		/* We have to fill the buffers with different data depending on whether one of the types was static. */
		const bool isKinematic = physics_get_type(hfirst) != PhysicsType::Static;
		const uint16 idx1 = isKinematic ? world->QueryInternalIndex(hfirst) : 0;
		const uint16 idx2 = world->QueryInternalIndex(hsecond);

		/* The first object might be static, so use zero for velocity instead of querying if it is. */
		const Vector3 v1 = isKinematic ? world->sysMove->GetVelocity(idx1) : Vector3();
		const Vector3 v2 = world->sysMove->GetVelocity(idx2);

		/* Angular velocity is handled in the same way as linear velocity. */
		const Vector3 w1 = isKinematic ? world->sysMove->GetAngularVelocity(idx1) : Vector3();
		const Vector3 w2 = world->sysMove->GetAngularVelocity(idx2);

		const Vector3 p2 = world->sysMove->GetPosition(world->QueryInternalHandle(hsecond));

		/* The mass scalars need to be properly set in the case of a kinematic collision. */
		if (isKinematic)
		{
			const Vector3 p1 = world->sysMove->GetPosition(world->QueryInternalHandle(hfirst));
			_mm256_seti_v3(tmp_p1, p1.X, p1.Y, p1.Z, k);
			tmp_imass1.V[k] = imass[hfirst];
			_mm256_seti_m3(tmp_moi1, imoi[hfirst].GetComponents(), k);
		}
		else
		{
			/*
			The position of the static object needs to be equal to the point of contact.
			This will make the relative velocity equal to the velocity of the second object
			at the contact point.
			*/
			_mm256_seti_v3(tmp_p1, world->sysCnst->px.get(i), world->sysCnst->py.get(i), world->sysCnst->pz.get(i), k);
			tmp_imass1.V[k] = 0.0f;
			_mm256_setzero_m3(tmp_moi1, k);
		}

		tmp_cor1.V[k] = mat1.CoR;
		tmp_cor2.V[k] = mat2.CoR;
		tmp_cof1.V[k] = mat1.CoFk;
		tmp_cof2.V[k] = mat2.CoFk;
		tmp_imass2.V[k] = imass[hsecond];
		_mm256_seti_v3(tmp_p2, p2.X, p2.Y, p2.Z, k);
		_mm256_seti_v3(tmp_v1, v1.X, v1.Y, v1.Z, k);
		_mm256_seti_v3(tmp_v2, v2.X, v2.Y, v2.Z, k);
		_mm256_seti_v3(tmp_w1, w1.Pitch, w1.Yaw, w1.Roll, k);
		_mm256_seti_v3(tmp_w2, w2.Pitch, w2.Yaw, w2.Roll, k);
		_mm256_seti_m3(tmp_moi2, imoi[hsecond].GetComponents(), k);

		/* Push the staging buffer to the output. */
		if (k >= 7 || i == size - 1)
		{
			cor1[j] = tmp_cor1.SIMD;
			cor2[j] = tmp_cor2.SIMD;
			cof1[j] = tmp_cof1.SIMD;
			cof2[j] = tmp_cof2.SIMD;
			_mm256_set1_v3(tmp_p1, px1[j], py1[j], pz1[j]);
			_mm256_set1_v3(tmp_p2, px2[j], py2[j], pz2[j]);
			_mm256_set1_v3(tmp_v1, vx1[j], vy1[j], vz1[j]);
			_mm256_set1_v3(tmp_v2, vx2[j], vy2[j], vz2[j]);
			_mm256_set1_v3(tmp_w1, wp1[j], wy1[j], wr1[j]);
			_mm256_set1_v3(tmp_w2, wp2[j], wy2[j], wr2[j]);
			imass1[j] = tmp_imass1.SIMD;
			imass2[j] = tmp_imass2.SIMD;
			_mm256_set1_m3(tmp_moi1, m001[j], m011[j], m021[j], m101[j], m111[j], m121[j], m201[j], m211[j], m221[j]);
			_mm256_set1_m3(tmp_moi2, m002[j], m012[j], m022[j], m102[j], m112[j], m122[j], m202[j], m212[j], m222[j]);
		}
	}
}
#pragma warning(pop)

/*
Values are per component (v in the comment is vx, vy and vz in code).
The order of these operations matters.
	Friction impulse is relative to linear impulse.
	Angular impulse is relative to linear + friction impulse.

Loop over all the AVX types (8 packed manifold) and solve them in parallel.
	Calculate the relative vector from the center of mass to the collision point (r1, r2).
	Calculate the relative velocity at the contact point (v).
	Calculate the linear collision impulse (j).
	Add linear impulse to result buffer, for both objects.
	Calculate the tangent of the collision (t).
	Calculate friction impulse (reuse j).
	Calculate angular impulse and apply it to both objects.
*/
void Pu::ContactSolverSystem::VectorSolve(ofloat dt)
{
	/* Predefine often used constants. */
	const size_t count = world->sysCnst->hfirsts.size();
	const size_t avxCnt = (count >> 0x3) + (count != 0);
	const ofloat zero = _mm256_setzero_ps();
	const ofloat one = _mm256_set1_ps(1.0f);
	const ofloat neg = _mm256_set1_ps(-1.0f);
	const ofloat beta = _mm256_set1_ps(PhysicsBaumgarteFactor);

	/* Cache pointers to the collision normal and contact point. */
	const ofloat *nx = world->sysCnst->nx.data();
	const ofloat *ny = world->sysCnst->ny.data();
	const ofloat *nz = world->sysCnst->nz.data();

	const ofloat *cx = world->sysCnst->px.data();
	const ofloat *cy = world->sysCnst->py.data();
	const ofloat *cz = world->sysCnst->pz.data();

	/* Use these as temporary vector buffers during various calculations. */
	ofloat tmp_x1, tmp_y1, tmp_z1;
	ofloat tmp_x2, tmp_y2, tmp_z2;
	ofloat tmp_x3, tmp_y3, tmp_z3;
	ofloat e, j, num, d1, d2;

	/* Solve collision per AVX type (i = second, k = first). */
	for (size_t i = 0, k = avxCnt; i < avxCnt; i++, k++)
	{
		/* Calculate the position of the first object relative to the contact point. */
		const ofloat rx1 = _mm256_sub_ps(cx[i], px1[i]);
		const ofloat ry1 = _mm256_sub_ps(cy[i], py1[i]);
		const ofloat rz1 = _mm256_sub_ps(cz[i], pz1[i]);

		/* Calculate the position of the second object relative to the contact point. */
		const ofloat rx2 = _mm256_sub_ps(cx[i], px2[i]);
		const ofloat ry2 = _mm256_sub_ps(cy[i], py2[i]);
		const ofloat rz2 = _mm256_sub_ps(cz[i], pz2[i]);

		/* Calculate the relative velocity of the objects. */
		_mm256_cross_v3(wp1[i], wy1[i], wr1[i], rx1, ry1, rz1, tmp_x1, tmp_y1, tmp_z1);
		_mm256_cross_v3(wp2[i], wy2[i], wr2[i], rx2, ry2, rz2, tmp_x2, tmp_y2, tmp_z2);
		const ofloat vx = _mm256_sub_ps(_mm256_add_ps(vx2[i], tmp_x2), _mm256_add_ps(vx1[i], tmp_x1));
		const ofloat vy = _mm256_sub_ps(_mm256_add_ps(vy2[i], tmp_y2), _mm256_add_ps(vy1[i], tmp_y1));
		const ofloat vz = _mm256_sub_ps(_mm256_add_ps(vz2[i], tmp_z2), _mm256_add_ps(vz1[i], tmp_z1));
		const ofloat vdn = _mm256_dot_v3(vx, vy, vz, nx[i], ny[i], nz[i]);

		/* Calculate and apply the normal force. */
		{
			/* Calculate the impulse. */
			e = _mm256_min_ps(cor1[i], cor2[i]);
			num = _mm256_mul_ps(_mm256_mul_ps(_mm256_add_ps(one, e), neg), vdn);
			_mm256_cross_v3(rx1, ry1, rz1, nx[i], ny[i], nz[i], tmp_x1, tmp_y1, tmp_z1);
			_mm256_mat3mul_v3(m001[i], m011[i], m021[i], m101[i], m111[i], m121[i], m201[i], m211[i], m221[i], tmp_x1, tmp_y1, tmp_z1, tmp_x2, tmp_y2, tmp_z2);
			d1 = _mm256_add_ps(imass1[i], _mm256_dot_v3(tmp_x1, tmp_y1, tmp_x1, tmp_x2, tmp_y2, tmp_z2));
			_mm256_cross_v3(rx2, ry2, rz2, nx[i], ny[i], nz[i], tmp_x1, tmp_y1, tmp_z1);
			_mm256_mat3mul_v3(m002[i], m012[i], m022[i], m102[i], m112[i], m122[i], m202[i], m212[i], m222[i], tmp_x1, tmp_y1, tmp_z1, tmp_x3, tmp_y3, tmp_z3);
			d2 = _mm256_add_ps(imass2[i], _mm256_dot_v3(tmp_x1, tmp_y1, tmp_z1, tmp_x3, tmp_y3, tmp_z3));
			j = _mm256_mul_ps(_mm256_divs_ps(num, _mm256_add_ps(d1, d2), zero), world->sysCnst->em[i]);

			/* Calculate the directional impulse. */
			tmp_x1 = _mm256_mul_ps(j, nx[i]);
			tmp_y1 = _mm256_mul_ps(j, ny[i]);
			tmp_z1 = _mm256_mul_ps(j, nz[i]);

			/* Apply the normal impulse to the first object. */
			jx[k] = _mm256_mul_ps(_mm256_mul_ps(tmp_x1, imass1[i]), neg);
			jy[k] = _mm256_mul_ps(_mm256_mul_ps(tmp_y1, imass1[i]), neg);
			jz[k] = _mm256_mul_ps(_mm256_mul_ps(tmp_z1, imass1[i]), neg);
			jpitch[k] = _mm256_mul_ps(_mm256_mul_ps(j, tmp_x2), neg);
			jyaw[k] = _mm256_mul_ps(_mm256_mul_ps(j, tmp_y2), neg);
			jroll[k] = _mm256_mul_ps(_mm256_mul_ps(j, tmp_z2), neg);

			/* Apply the normal impulse to the second object. */
			jx[i] = _mm256_mul_ps(tmp_x1, imass2[i]);
			jy[i] = _mm256_mul_ps(tmp_y1, imass2[i]);
			jz[i] = _mm256_mul_ps(tmp_z1, imass2[i]);
			jpitch[i] = _mm256_mul_ps(j, tmp_x3);
			jyaw[i] = _mm256_mul_ps(j, tmp_y3);
			jroll[i] = _mm256_mul_ps(j, tmp_z3);

			/* Stabalize using Baumgarte. */
			d1 = _mm256_mul_ps(_mm256_div_ps(beta, world->sysCnst->em[i]), _mm256_div_ps(world->sysCnst->sd[i], dt));
			tmp_x1 = _mm256_mul_ps(d1, nx[i]);
			tmp_y1 = _mm256_mul_ps(d1, ny[i]);
			tmp_z1 = _mm256_mul_ps(d1, nz[i]);
			jx[k] = _mm256_sub_ps(jx[k], tmp_x1);
			jy[k] = _mm256_sub_ps(jy[k], tmp_y1);
			jz[k] = _mm256_sub_ps(jz[k], tmp_z1);
			jx[i] = _mm256_add_ps(jx[i], tmp_x1);
			jy[i] = _mm256_add_ps(jy[i], tmp_y1);
			jz[i] = _mm256_add_ps(jz[i], tmp_z1);
		}

		/* Calculate and apply the kinetic friction force. */
		{
			/* Calcualte the tangent of the collision. */
			ofloat tx = _mm256_sub_ps(vx, _mm256_mul_ps(vdn, nx[i]));
			ofloat ty = _mm256_sub_ps(vy, _mm256_mul_ps(vdn, ny[i]));
			ofloat tz = _mm256_sub_ps(vz, _mm256_mul_ps(vdn, nz[i]));
			_mm256_norm_v3(tx, ty, tz, zero);

			/* Calculate the impulse. */
			e = _mm256_sqrt_ps(_mm256_mul_ps(cof1[i], cof2[i]));
			num = _mm256_mul_ps(_mm256_dot_v3(vx, vy, vz, tx, ty, tz), neg);
			_mm256_cross_v3(rx1, ry1, rz1, tx, ty, tz, tmp_x1, tmp_y1, tmp_z1);
			_mm256_mat3mul_v3(m001[i], m011[i], m021[i], m101[i], m111[i], m121[i], m201[i], m211[i], m221[i], tmp_x1, tmp_y1, tmp_z1, tmp_x2, tmp_y2, tmp_z2);
			d1 = _mm256_add_ps(imass1[i], _mm256_dot_v3(tmp_x1, tmp_y1, tmp_x1, tmp_x2, tmp_y2, tmp_z2));
			_mm256_cross_v3(rx2, ry2, rz2, tx, ty, tz, tmp_x1, tmp_y1, tmp_z1);
			_mm256_mat3mul_v3(m002[i], m012[i], m022[i], m102[i], m112[i], m122[i], m202[i], m212[i], m222[i], tmp_x1, tmp_y1, tmp_z1, tmp_x3, tmp_y3, tmp_z3);
			d2 = _mm256_add_ps(imass2[i], _mm256_dot_v3(tmp_x1, tmp_y1, tmp_z1, tmp_x3, tmp_y3, tmp_z3));
			j = _mm256_clamp_ps(_mm256_divs_ps(num, _mm256_add_ps(d1, d2), zero), _mm256_mul_ps(_mm256_mul_ps(e, neg), j), _mm256_mul_ps(e, j));

			/* Calculate the directional impulse. */
			tmp_x1 = _mm256_mul_ps(j, tx);
			tmp_y1 = _mm256_mul_ps(j, ty);
			tmp_z1 = _mm256_mul_ps(j, tz);

			/* Apply the kinetic friction impulse to the first object. */
			jx[k] = _mm256_sub_ps(jx[k], _mm256_mul_ps(tmp_x1, imass1[i]));
			jy[k] = _mm256_sub_ps(jy[k], _mm256_mul_ps(tmp_y1, imass1[i]));
			jz[k] = _mm256_sub_ps(jz[k], _mm256_mul_ps(tmp_z1, imass1[i]));
			jpitch[k] = _mm256_sub_ps(jpitch[k], _mm256_mul_ps(j, tmp_x2));
			jyaw[k] = _mm256_sub_ps(jyaw[k], _mm256_mul_ps(j, tmp_y2));
			jroll[k] = _mm256_sub_ps(jroll[k], _mm256_mul_ps(j, tmp_z2));

			/* Apply the kinetic friction impulse to the second object. */
			jx[i] = _mm256_add_ps(jx[i], _mm256_mul_ps(tmp_x1, imass2[i]));
			jy[i] = _mm256_add_ps(jy[i], _mm256_mul_ps(tmp_y1, imass2[i]));
			jz[i] = _mm256_add_ps(jz[i], _mm256_mul_ps(tmp_z1, imass2[i]));
			jpitch[i] = _mm256_add_ps(jpitch[i], _mm256_mul_ps(j, tmp_x3));
			jyaw[i] = _mm256_add_ps(jyaw[i], _mm256_mul_ps(j, tmp_y3));
			jroll[i] = _mm256_add_ps(jroll[i], _mm256_mul_ps(j, tmp_z3));
		}
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

		float pitch = AVX_FLOAT_UNION{ jpitch[j] }.V[k];
		float yaw = AVX_FLOAT_UNION{ jyaw[j] }.V[k];
		float roll = AVX_FLOAT_UNION{ jroll[j] }.V[k];

		world->sysMove->AddForce(world->QueryInternalIndex(hsecond), x, y, z, pitch, yaw, roll);

		/* Add the total applied impulse to the debugging list. */
#ifdef _DEBUG
		const Vector3 at{ world->sysCnst->px.get(i), world->sysCnst->py.get(i), world->sysCnst->pz.get(i) };

		Vector3 f{ x, y, z };
		float mag = f.Length();
		DBG_ADD_FORCE();
#endif

		/* The second object might not need impulses to be applied. */
		if (physics_get_type(hfirst) != PhysicsType::Static)
		{
			x = AVX_FLOAT_UNION{ jx[j + avxCnt] }.V[k];
			y = AVX_FLOAT_UNION{ jy[j + avxCnt] }.V[k];
			z = AVX_FLOAT_UNION{ jz[j + avxCnt] }.V[k];

			pitch = AVX_FLOAT_UNION{ jpitch[j + avxCnt] }.V[k];
			yaw = AVX_FLOAT_UNION{ jyaw[j + avxCnt] }.V[k];
			roll = AVX_FLOAT_UNION{ jroll[j + avxCnt] }.V[k];

			world->sysMove->AddForce(world->QueryInternalIndex(hfirst), x, y, z, pitch, yaw, roll);

#ifdef _DEBUG
			f.X = x;
			f.Y = y;
			f.Z = z;
			mag = f.Length();
			DBG_ADD_FORCE();
#endif
		}
	}
}

void Pu::ContactSolverSystem::Destroy(void)
{
	if (capacity)
	{
		/* Free the coeffiecients. */
		_mm256_free_ps(cor1);
		_mm256_free_ps(cor2);
		_mm256_free_ps(cof1);
		_mm256_free_ps(cof2);

		/* Free the positions. */
		_mm256_free_v3(px1, py1, pz1);
		_mm256_free_v3(px2, py2, pz2);

		/* Free the velocities. */
		_mm256_free_v3(vx1, vy1, vz1);
		_mm256_free_v3(vx2, vy2, vz2);

		/* Free the angular velocities. */
		_mm256_free_v3(wp1, wy1, wr1);
		_mm256_free_v3(wp2, wy2, wr2);

		/* Free the inverse masses. */
		_mm256_free_ps(imass1);
		_mm256_free_ps(imass2);

		/* Free the moment of inertia tensors. */
		_mm256_free_m3(m001, m011, m021, m101, m111, m121, m201, m211, m221);
		_mm256_free_m3(m002, m012, m022, m102, m112, m122, m202, m212, m222);

		/* Free the result impulses. */
		_mm256_free_v3(jx, jy, jz);
		_mm256_free_v3(jpitch, jyaw, jroll);
	}
}