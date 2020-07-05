#include "Physics/Systems/MovementSystem.h"
#include "Config.h"

Pu::MovementSystem::MovementSystem(void)
	: g(0.0f, -9.81f, 0.0f)
{}

void Pu::MovementSystem::AddForce(size_t idx, Vector3 linear, Quaternion angular)
{
	/* Add the linear force to the velocity. */
	vx.add(idx, linear.X);
	vy.add(idx, linear.Y);
	vz.add(idx, linear.Z);

	/* Add the angular force to the rotation. */
	wi.add(idx, angular.I);
	wj.add(idx, angular.J);
	wk.add(idx, angular.K);
	wr.add(idx, angular.R);
}

size_t Pu::MovementSystem::AddItem(Vector3 p, Vector3 v, Quaternion theta, Quaternion omega, float CoD, float imass)
{
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
	}
}

void Pu::MovementSystem::ApplyGravity(ofloat dt)
{
	for (ofloat &x : vx) x = _mm256_add_ps(x, _mm256_mul_ps(g.X, dt));
	for (ofloat &y : vy) y = _mm256_add_ps(y, _mm256_mul_ps(g.Y, dt));
	for (ofloat &z : vz) z = _mm256_add_ps(z, _mm256_mul_ps(g.Z, dt));
}

void Pu::MovementSystem::ApplyDrag(ofloat dt)
{
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
}

void Pu::MovementSystem::Integrate(ofloat dt)
{
	const size_t size = vx.sse_size();

	/* Add linear velocity to position (scaled by delta time). */
	for (size_t i = 0; i < size; i++) px[i] = _mm256_add_ps(px[i], _mm256_mul_ps(vx[i], dt));
	for (size_t i = 0; i < size; i++) py[i] = _mm256_add_ps(py[i], _mm256_mul_ps(vy[i], dt));
	for (size_t i = 0; i < size; i++) pz[i] = _mm256_add_ps(pz[i], _mm256_mul_ps(vz[i], dt));

	/* Add angular rotation to orientation (scaled by delta time). */
	for (size_t i = 0; i < size; i++) ti[i] = _mm256_add_ps(ti[i], _mm256_mul_ps(wi[i], dt));
	for (size_t i = 0; i < size; i++) tj[i] = _mm256_add_ps(tj[i], _mm256_mul_ps(wj[i], dt));
	for (size_t i = 0; i < size; i++) tk[i] = _mm256_add_ps(tk[i], _mm256_mul_ps(wk[i], dt));
	for (size_t i = 0; i < size; i++) tr[i] = _mm256_add_ps(tr[i], _mm256_mul_ps(wr[i], dt));
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

void Pu::MovementSystem::CheckDistance(vector<std::pair<size_t, Vector3>> & result) const
{
	const size_t size = vx.sse_size();

	/* 
	Pre-calculate the square distance.
	The expansion is done using an inflate operation.
	So the maximum distance in any direction is half of the actual expansion.
	*/
	ofloat maxDist = _mm256_set1_ps(KinematicExpansion * 0.5f);
	maxDist = _mm256_mul_ps(maxDist, maxDist);

	AVX_FLOAT_UNION *masks = reinterpret_cast<AVX_FLOAT_UNION*>(_aligned_malloc(sizeof(ofloat) * size, sizeof(ofloat)));
	for (size_t i = 0; i < size; i++)
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
		const AVX_FLOAT_UNION cur = masks[i];
		for (size_t j = 0; j < 8; j++)
		{
			const size_t idx = i << 0x3 | j;
			if (idx >= px.size()) break;

			if (cur.V[i])
			{
				const float x = px.get(idx);
				const float y = py.get(idx);
				const float z = pz.get(idx);

				/* The old location needs to be overriden when it reaches this point. */
				qx.set(idx, x);
				qy.set(idx, y);
				qz.set(idx, z);

				/* Return both the index and the new location. */
				result.emplace_back(std::make_pair(idx, Vector3(x, y, z)));
			}
		}
	}

	_aligned_free(masks);
}