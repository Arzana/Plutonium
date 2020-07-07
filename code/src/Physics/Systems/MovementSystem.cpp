#include "Physics/Systems/MovementSystem.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Core/Diagnostics/Profiler.h"
#include "Config.h"

#define alloc_tmp(size)		reinterpret_cast<Pu::AVX_FLOAT_UNION*>(_aligned_malloc(sizeof(Pu::ofloat) * size, sizeof(Pu::ofloat)))

Pu::MovementSystem::MovementSystem(void)
	: g(0.0f, -9.81f, 0.0f)
{}

void Pu::MovementSystem::AddForce(size_t idx, float x, float y, float z, Quaternion angular)
{
	/* Add the linear force to the velocity. */
	vx.add(idx, x);
	vy.add(idx, y);
	vz.add(idx, z);

	/* Add the angular force to the rotation. */
	wi.add(idx, angular.I);
	wj.add(idx, angular.J);
	wk.add(idx, angular.K);
	wr.add(idx, angular.R);

	/* Save this impulse on debug mode, so we can display it. */
#ifdef _DEBUG
	if (addForces)
	{
		const Vector3 at = Vector3(px.get(idx), py.get(idx), pz.get(idx));
		const Vector3 force = Vector3(x, y, z);
		forces.emplace_back(TimedForce{ ImpulseDebuggingTTL, force.Length(), at, normalize(force) });
	}
#endif
}

size_t Pu::MovementSystem::AddItem(Vector3 p, Vector3 v, Quaternion theta, Quaternion omega, float CoD, float imass)
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

	cod.push(CoD);
	m.push(imass);
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
	wi.push(omega.I);
	wj.push(omega.J);
	wk.push(omega.K);
	wr.push(omega.R);

	qx.push(p.X);
	qy.push(p.Y);
	qz.push(p.Z);
	sleep.push(cnvrt.f);

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
		wi.erase(idx);
		wj.erase(idx);
		wk.erase(idx);
		wr.erase(idx);

		qx.erase(idx);
		qy.erase(idx);
		qz.erase(idx);
		sleep.erase(idx);
	}
}

void Pu::MovementSystem::ApplyGravity(ofloat dt)
{
	if constexpr (PhysicsProfileSystems) Profiler::Begin("Movement", Color::Gray());

	const size_t size = vx.sse_size();
	for (size_t i = 0; i < size; i++) vx[i] = _mm256_and_ps(_mm256_add_ps(vx[i], _mm256_mul_ps(g.X, dt)), sleep[i]);
	for (size_t i = 0; i < size; i++) vy[i] = _mm256_and_ps(_mm256_add_ps(vy[i], _mm256_mul_ps(g.Y, dt)), sleep[i]);
	for (size_t i = 0; i < size; i++) vz[i] = _mm256_and_ps(_mm256_add_ps(vz[i], _mm256_mul_ps(g.Z, dt)), sleep[i]);

	if constexpr (PhysicsProfileSystems) Profiler::End();
}

void Pu::MovementSystem::ApplyDrag(ofloat dt)
{
	if constexpr (PhysicsProfileSystems) Profiler::Begin("Movement", Color::Gray());

	const size_t size = vx.sse_size();
	const ofloat zero = _mm256_set1_ps(0.0f);

	for (size_t i = 0; i < size; i++)
	{
		ofloat &x = vx[i];
		ofloat &y = vy[i];
		ofloat &z = vz[i];

		const ofloat xx = _mm256_mul_ps(x, x);
		const ofloat yy = _mm256_mul_ps(y, y);
		const ofloat zz = _mm256_mul_ps(z, z);

		/*
		Calculate the following values:
		- Square magnitude of velocity (ll).
		- Magnitude of velocity (l).
		- Aerodynamic drag scalar (ld).
		*/
		const ofloat ll = _mm256_add_ps(_mm256_add_ps(xx, yy), zz);
		const ofloat l = _mm256_andnot_ps(_mm256_cmp_ps(zero, ll, _CMP_EQ_OQ), _mm256_rsqrt_ps(ll));
		const ofloat ld = _mm256_mul_ps(ll, cod[i]);

		/* Calculate the aerodynamic force to apply. */
		const ofloat fx = _mm256_mul_ps(_mm256_mul_ps(x, l), ld);
		const ofloat fy = _mm256_mul_ps(_mm256_mul_ps(y, l), ld);
		const ofloat fz = _mm256_mul_ps(_mm256_mul_ps(z, l), ld);

		/* Apply the force scaled with delta time. */
		x = _mm256_sub_ps(x, _mm256_mul_ps(fx, _mm256_mul_ps(m[i], dt)));
		y = _mm256_sub_ps(y, _mm256_mul_ps(fy, _mm256_mul_ps(m[i], dt)));
		z = _mm256_sub_ps(z, _mm256_mul_ps(fz, _mm256_mul_ps(m[i], dt)));
	}

	if constexpr (PhysicsProfileSystems) Profiler::End();
}

void Pu::MovementSystem::Integrate(ofloat dt)
{
	if constexpr (PhysicsProfileSystems) Profiler::Begin("Movement", Color::Gray());
	const size_t size = vx.sse_size();

#ifdef _DEBUG
	addForces = false;
#endif

	/* Add linear velocity to position (scaled by delta time). */
	for (size_t i = 0; i < size; i++) px[i] = _mm256_add_ps(px[i], _mm256_mul_ps(vx[i], dt));
	for (size_t i = 0; i < size; i++) py[i] = _mm256_add_ps(py[i], _mm256_mul_ps(vy[i], dt));
	for (size_t i = 0; i < size; i++) pz[i] = _mm256_add_ps(pz[i], _mm256_mul_ps(vz[i], dt));

	/* Add angular rotation to orientation (scaled by delta time). */
	for (size_t i = 0; i < size; i++) ti[i] = _mm256_add_ps(ti[i], _mm256_mul_ps(wi[i], dt));
	for (size_t i = 0; i < size; i++) tj[i] = _mm256_add_ps(tj[i], _mm256_mul_ps(wj[i], dt));
	for (size_t i = 0; i < size; i++) tk[i] = _mm256_add_ps(tk[i], _mm256_mul_ps(wk[i], dt));
	for (size_t i = 0; i < size; i++) tr[i] = _mm256_add_ps(tr[i], _mm256_mul_ps(wr[i], dt));

	if constexpr (PhysicsProfileSystems) Profiler::End();
}

Pu::Matrix Pu::MovementSystem::GetTransform(PhysicsHandle handle) const
{
	const uint16 idx = physics_get_lookup_id(handle);

	/* Static objects are cached as they don't move. */
	if (physics_get_type(handle) == PhysicsType::Static) return transforms[idx];

	const Matrix translation = Matrix::CreateTranslation(px.get(idx), py.get(idx), pz.get(idx));
	const Matrix orientation = Matrix::CreateRotation(Quaternion(tr.get(idx), ti.get(idx), tj.get(idx), tk.get(idx)));
	return translation * orientation;
}

Pu::Vector3 Pu::MovementSystem::GetVelocity(size_t idx) const
{
	return Vector3(vx.get(idx), vy.get(idx), vz.get(idx));
}

/*
foreach object
	if distance(p, q) > max distance
		q = p
		add current object to result
*/
void Pu::MovementSystem::CheckDistance(vector<std::pair<size_t, Vector3>> & result) const
{
	const size_t size_avx = vx.sse_size();
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
		const ofloat d = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(sx, sx), _mm256_mul_ps(sy, sy)), _mm256_mul_ps(sz, sz));

		/* Check whether the distance is greater than the maximum. */
		masks[i].AVX = _mm256_cmp_ps(d, maxDist, _CMP_GT_OQ);
	}

	/* Convert the AVX type to normal floats and add them to the result buffer. */
	for (size_t i = 0; i < size; i++)
	{
		const size_t j = i >> 0x3;
		const size_t k = i & 0x7;

		if (masks[j].V[k])
		{
			const float x = px.get(i);
			const float y = py.get(i);
			const float z = pz.get(i);

			/* The old location needs to be overriden when it reaches this point. */
			qx.set(i, x);
			qy.set(i, y);
			qz.set(i, z);

			/* Return both the index and the new location. */
			result.emplace_back(std::make_pair(i, Vector3(x, y, z)));
		}
	}

	_aligned_free(masks);
}

void Pu::MovementSystem::TrySleep(ofloat epsilon)
{
	if constexpr (PhysicsAllowSleeping)
	{
		if constexpr (PhysicsProfileSystems) Profiler::Begin("Movement", Color::Gray());

		/* Predefine the minimum distance for us to consider the object sleepable. */
		const size_t size_avx = vx.sse_size();
		const size_t size = vx.size();
		const ofloat minMag = _mm256_mul_ps(epsilon, epsilon);

		/* Populate a temporary buffer with the compared distances. */
		for (size_t i = 0; i < size_avx; i++)
		{
			const ofloat d = _mm256_add_ps(_mm256_mul_ps(vx[i], vx[i]), _mm256_add_ps(_mm256_mul_ps(vy[i], vy[i]), _mm256_mul_ps(vz[i], vz[i])));
			sleep[i] = _mm256_cmp_ps(d, minMag, _CMP_GT_OQ);
		}

		if constexpr (PhysicsProfileSystems) Profiler::End();
	}
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