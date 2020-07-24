#include "Physics/Systems/MovementSystem.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/Vector3_SIMD.h"
#include "Core/Math/Vector4_SIMD.h"
#include "Config.h"

#define alloc_tmp(size)		reinterpret_cast<Pu::AVX_FLOAT_UNION*>(_aligned_malloc(sizeof(Pu::ofloat) * size, sizeof(Pu::ofloat)))

Pu::MovementSystem::MovementSystem(void)
	: Gx(_mm256_setzero_ps()), Gy(_mm256_set1_ps(-9.81f)), Gz(_mm256_setzero_ps())
{}

void Pu::MovementSystem::AddForce(size_t idx, float x, float y, float z, float pitch, float yaw, float roll)
{
	/* Add the linear force to the velocity. */
	vx.add(idx, x);
	vy.add(idx, y);
	vz.add(idx, z);

	/* Add the angular force to the rotation. */
	wp.add(idx, pitch);
	wy.add(idx, yaw);
	wr.add(idx, roll);

	/* Save this impulse on debug mode, so we can display it. */
#ifdef _DEBUG
	if (addForces)
	{
		const Vector3 at = Vector3(px.get(idx), py.get(idx), pz.get(idx));
		const Vector3 force = Vector3(x, y, z);
		forces.emplace_back(TimedForce{ PhysicsDebuggingTTL, force.Length(), at, normalize(force) });
	}
#endif
}

size_t Pu::MovementSystem::AddItem(Vector3 p, Vector3 v, Quaternion theta, Vector3 omega, Vector3 scale, float CoD, float imass, const Matrix3 &moi)
{
	/* 
	We need to put a mask of all ones into the affected by gravity buffer. 
	So use an union to create that constant.
	*/
	static union
	{
		uint32 u;
		float f;
	} cnvrt{ ~0u };

	const float *mat = moi.GetComponents();

	cod.push(CoD);
	m.push(imass);
	m00.push(mat[0]);
	m01.push(mat[1]);
	m02.push(mat[2]);
	m10.push(mat[3]);
	m11.push(mat[4]);
	m12.push(mat[5]);
	m20.push(mat[6]);
	m21.push(mat[7]);
	m22.push(mat[8]);

	px.push(p.X);
	py.push(p.Y);
	pz.push(p.Z);
	vx.push(v.X);
	vy.push(v.Y);
	vz.push(v.Z);
	ti.push(theta.I);
	tj.push(theta.J);
	tk.push(theta.K);
	tr.push(theta.R);
	wp.push(omega.Pitch);
	wy.push(omega.Yaw);
	wr.push(omega.Roll);

	qx.push(p.X);
	qy.push(p.Y);
	qz.push(p.Z);
	sleep.push(cnvrt.f);
	scales.emplace_back(scale);

	return cod.size() - 1;
}

size_t Pu::MovementSystem::AddItem(const Matrix & transform)
{
	transforms.emplace_back(transform);
	return transforms.size() - 1;
}

void Pu::MovementSystem::RemoveItem(PhysicsHandle handle)
{
	const uint16 idx = physics_get_lookup_id(handle);

	if (physics_get_type(handle) == PhysicsType::Static) transforms.removeAt(idx);
	else
	{
		cod.erase(idx);
		m.erase(idx);
		px.erase(idx);
		py.erase(idx);
		pz.erase(idx);
		vx.erase(idx);
		vy.erase(idx);
		vz.erase(idx);
		ti.erase(idx);
		tj.erase(idx);
		tk.erase(idx);
		tr.erase(idx);
		wp.erase(idx);
		wy.erase(idx);
		wr.erase(idx);

		qx.erase(idx);
		qy.erase(idx);
		qz.erase(idx);
		sleep.erase(idx); 
		scales.removeAt(idx);
	}
}

void Pu::MovementSystem::ApplyGravity(ofloat dt)
{
	if constexpr (ProfileWorldSystems) Profiler::Begin("Movement", Color::Gray());

	const size_t size = vx.simd_size();
	for (size_t i = 0; i < size; i++) vx[i] = _mm256_and_ps(_mm256_add_ps(vx[i], _mm256_mul_ps(Gx, dt)), sleep[i]);
	for (size_t i = 0; i < size; i++) vy[i] = _mm256_and_ps(_mm256_add_ps(vy[i], _mm256_mul_ps(Gy, dt)), sleep[i]);
	for (size_t i = 0; i < size; i++) vz[i] = _mm256_and_ps(_mm256_add_ps(vz[i], _mm256_mul_ps(Gz, dt)), sleep[i]);

	if constexpr (ProfileWorldSystems) Profiler::End();
}

void Pu::MovementSystem::ApplyDrag(ofloat dt)
{
	if constexpr (ProfileWorldSystems) Profiler::Begin("Movement", Color::Gray());

	const size_t size = vx.simd_size();
	const ofloat zero = _mm256_set1_ps(0.0f);

	ofloat xx, yy, zz;
	ofloat ll, l, ld;
	ofloat fx, fy, fz;

	/* Apply linear drag. */
	for (size_t i = 0; i < size; i++)
	{
		ofloat &x = vx[i];
		ofloat &y = vy[i];
		ofloat &z = vz[i];

		xx = _mm256_mul_ps(x, x);
		yy = _mm256_mul_ps(y, y);
		zz = _mm256_mul_ps(z, z);

		/*
		Calculate the following values:
		- Square magnitude of velocity (ll).
		- Magnitude of velocity (l).
		- Aerodynamic drag scalar (ld).
		*/
		ll = _mm256_add_ps(_mm256_add_ps(xx, yy), zz);
		l = _mm256_andnot_ps(_mm256_cmp_ps(zero, ll, _CMP_EQ_OQ), _mm256_rsqrt_ps(ll));
		ld = _mm256_mul_ps(ll, cod[i]);

		/* Calculate the aerodynamic force to apply. */
		fx = _mm256_mul_ps(_mm256_mul_ps(x, l), ld);
		fy = _mm256_mul_ps(_mm256_mul_ps(y, l), ld);
		fz = _mm256_mul_ps(_mm256_mul_ps(z, l), ld);

		/* Apply the force scaled with delta time. */
		x = _mm256_sub_ps(x, _mm256_mul_ps(fx, _mm256_mul_ps(m[i], dt)));
		y = _mm256_sub_ps(y, _mm256_mul_ps(fy, _mm256_mul_ps(m[i], dt)));
		z = _mm256_sub_ps(z, _mm256_mul_ps(fz, _mm256_mul_ps(m[i], dt)));
	}

	/* Apply angular drag. */
	for (size_t i = 0; i < size; i++)
	{
		ofloat &pitch = wp[i];
		ofloat &yaw = wy[i];
		ofloat &roll = wr[i];

		xx = _mm256_mul_ps(pitch, pitch);
		yy = _mm256_mul_ps(yaw, yaw);
		zz = _mm256_mul_ps(roll, roll);

		ll = _mm256_add_ps(_mm256_add_ps(xx, yy), zz);
		l = _mm256_andnot_ps(_mm256_cmp_ps(zero, ll, _CMP_EQ_OQ), _mm256_rsqrt_ps(ll));
		ld = _mm256_mul_ps(ll, cod[i]);

		const ofloat fx2 = _mm256_mul_ps(_mm256_mul_ps(pitch, l), ld);
		const ofloat fy2 = _mm256_mul_ps(_mm256_mul_ps(yaw, l), ld);
		const ofloat fz2 = _mm256_mul_ps(_mm256_mul_ps(roll, l), ld);

		/* Multiply by the moment of inertia tensor (inline mat3 * vec3). */
		fx = _mm256_add_ps(_mm256_mul_ps(fx2, m00[i]), _mm256_add_ps(_mm256_mul_ps(fy2, m10[i]), _mm256_mul_ps(fz2, m20[i])));
		fy = _mm256_add_ps(_mm256_mul_ps(fx2, m01[i]), _mm256_add_ps(_mm256_mul_ps(fy2, m11[i]), _mm256_mul_ps(fz2, m21[i])));
		fz = _mm256_add_ps(_mm256_mul_ps(fx2, m02[i]), _mm256_add_ps(_mm256_mul_ps(fy2, m12[i]), _mm256_mul_ps(fz2, m22[i])));

		pitch = _mm256_sub_ps(pitch, _mm256_mul_ps(fx, dt));
		yaw = _mm256_sub_ps(yaw, _mm256_mul_ps(fy, dt));
		roll = _mm256_sub_ps(roll, _mm256_mul_ps(fz, dt));
	}

	if constexpr (ProfileWorldSystems) Profiler::End();
}

void Pu::MovementSystem::Integrate(ofloat dt)
{
	if constexpr (ProfileWorldSystems) Profiler::Begin("Movement", Color::Gray());

	const size_t size = vx.simd_size();
	const ofloat zero = _mm256_setzero_ps();
	const ofloat half = _mm256_set1_ps(0.5f);
	const ofloat neg = _mm256_set1_ps(-1.0f);

#ifdef _DEBUG
	addForces = false;
#endif

	/* Add linear velocity to position (scaled by delta time). */
	for (size_t i = 0; i < size; i++) px[i] = _mm256_add_ps(px[i], _mm256_mul_ps(vx[i], dt));
	for (size_t i = 0; i < size; i++) py[i] = _mm256_add_ps(py[i], _mm256_mul_ps(vy[i], dt));
	for (size_t i = 0; i < size; i++) pz[i] = _mm256_add_ps(pz[i], _mm256_mul_ps(vz[i], dt));

	for (size_t i = 0; i < size; i++)
	{
		/* Convert angular velocity into quaterion for (R = 0).*/
		const ofloat qi = _mm256_mul_ps(_mm256_mul_ps(wp[i], dt), half);
		const ofloat qj = _mm256_mul_ps(_mm256_mul_ps(wy[i], dt), half);
		const ofloat qk = _mm256_mul_ps(_mm256_mul_ps(wr[i], dt), half);

		/* Quaternion multiplication in AVX (R = 0). */
		const ofloat dr = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(qi, ti[i]), _mm256_add_ps(_mm256_mul_ps(qj, tj[i]), _mm256_mul_ps(qk, tk[i]))), neg);
		const ofloat di = _mm256_add_ps(_mm256_mul_ps(qi, tr[i]), _mm256_sub_ps(_mm256_mul_ps(qj, tk[i]), _mm256_mul_ps(qk, tj[i])));
		const ofloat dj = _mm256_add_ps(_mm256_mul_ps(qj, tr[i]), _mm256_sub_ps(_mm256_mul_ps(qk, ti[i]), _mm256_mul_ps(qi, tk[i])));
		const ofloat dk = _mm256_add_ps(_mm256_mul_ps(qk, tr[i]), _mm256_sub_ps(_mm256_mul_ps(qi, tj[i]), _mm256_mul_ps(qj, ti[i])));

		/* Add angular velocity to orientation. */
		ti[i] = _mm256_add_ps(ti[i], di);
		tj[i] = _mm256_add_ps(tj[i], dj);
		tk[i] = _mm256_add_ps(tk[i], dk);
		tr[i] = _mm256_add_ps(tr[i], dr);

		/* 
		Normalize orientation. 
		Make sure to do this after and not before applying angular velocity.
		*/
		_mm256_norm_v4(ti[i], tj[i], tk[i], tr[i], zero);
	}

	if constexpr (ProfileWorldSystems) Profiler::End();
}

Pu::Matrix Pu::MovementSystem::GetTransform(PhysicsHandle handle) const
{
	const uint16 idx = physics_get_lookup_id(handle);

	/* Static objects are cached as they don't move. */
	if (physics_get_type(handle) == PhysicsType::Static) return transforms[idx];

	const Vector3 translation{ px.get(idx), py.get(idx), pz.get(idx) };
	const Quaternion orientation{ tr.get(idx), ti.get(idx), tj.get(idx), tk.get(idx) };
	return Matrix::CreateWorld(translation, orientation, scales[idx]);
}

Pu::Vector3 Pu::MovementSystem::GetPosition(PhysicsHandle handle) const
{
	const uint16 idx = physics_get_lookup_id(handle);
	
	/* Static objects don't have their position saved, so get it from the transform. */
	if (physics_get_type(handle) == PhysicsType::Static) return transforms[idx].GetTranslation();

	return Vector3(px.get(idx), py.get(idx), pz.get(idx));
}

Pu::Vector3 Pu::MovementSystem::GetVelocity(size_t idx) const
{
	return Vector3(vx.get(idx), vy.get(idx), vz.get(idx));
}

Pu::Vector3 Pu::MovementSystem::GetAngularVelocity(size_t idx) const
{
	return Vector3(wp.get(idx), wy.get(idx), wr.get(idx));
}

/*
foreach object
	if distance(p, q) > max distance
		q = p
		add current object to result
*/
void Pu::MovementSystem::CheckDistance(vector<size_t> & result) const
{
	const size_t size_avx = vx.simd_size();
	const size_t size = vx.size();

	/*
	Pre-calculate the square distance.
	The expansion is done using an inflate operation.
	So the maximum distance in any direction is half of the actual expansion.
	*/
	const ofloat maxDist = _mm256_set1_ps(sqr(KinematicExpansion * 0.5f));

	AVX_FLOAT_UNION *masks = alloc_tmp(size_avx);
	for (size_t i = 0; i < size_avx; i++)
	{
		/* Calculate the distance between the current position and the previous. */
		const ofloat sx = _mm256_sub_ps(qx[i], px[i]);
		const ofloat sy = _mm256_sub_ps(qy[i], py[i]);
		const ofloat sz = _mm256_sub_ps(qz[i], pz[i]);
		const ofloat d = _mm256_len2_v3(sx, sy, sz);

		/* Check whether the distance is greater than the maximum. */
		masks[i].SIMD = _mm256_cmp_ps(d, maxDist, _CMP_GT_OQ);
	}

	/* Convert the AVX type to normal floats and add them to the result buffer. */
	for (size_t i = 0; i < size; i++)
	{
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		if (masks[j].V[k])
		{
			/* The old location needs to be overriden when it reaches this point. */
			qx.set(i, px.get(i));
			qy.set(i, py.get(i));
			qz.set(i, pz.get(i));

			/* Return both the index and the new location. */
			result.emplace_back(i);
		}
	}

	_aligned_free(masks);
}

void Pu::MovementSystem::TrySleep(ofloat epsilon)
{
	if constexpr (PhysicsAllowSleeping)
	{
		if constexpr (ProfileWorldSystems) Profiler::Begin("Movement", Color::Gray());

		/* Predefine the minimum distance for us to consider the object sleepable. */
		const size_t size_avx = vx.simd_size();
		const size_t size = vx.size();
		const ofloat minMag = _mm256_mul_ps(epsilon, epsilon);

		/* Populate a temporary buffer with the compared distances. */
		for (size_t i = 0; i < size_avx; i++)
		{
			const ofloat d = _mm256_len2_v3(vx[i], vy[i], vz[i]);
			sleep[i] = _mm256_cmp_ps(d, minMag, _CMP_GT_OQ);
		}

		if constexpr (ProfileWorldSystems) Profiler::End();
	}
}

size_t Pu::MovementSystem::GetSleepingCount(void) const
{
	size_t result = sleep.size();

	for (ofloat cur : sleep)
	{
		result -= _mm_popcnt_u32(static_cast<uint32>(_mm256_movemask_ps(cur)));
	}

	return result;
}

#ifdef _DEBUG
void Pu::MovementSystem::Visualize(DebugRenderer & dbgRenderer, float dt) const
{
	addForces = true;

	/* Remove any old impulses. from the list. */
	for (size_t i = 0; i < forces.size();)
	{
		if ((forces[i].TTL -= dt) <= 0.0f) forces.removeAt(i);
		else i++;
	}

	/* Display the forces. */
	for (const TimedForce &force : forces)
	{
		dbgRenderer.AddArrow(force.Position, force.Direction, Color::WhiteSmoke(), force.Magnitude);
	}
}
#endif