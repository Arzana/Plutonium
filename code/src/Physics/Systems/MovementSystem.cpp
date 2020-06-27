#include "Physics/Systems/MovementSystem.h"

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

	return cod.size() - 1;
}

void Pu::MovementSystem::RemoveItem(size_t idx)
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
}

void Pu::MovementSystem::ApplyGravity(ofloat dt)
{
	for (ofloat &x : px) x = _mm256_mul_ps(_mm256_mul_ps(x, g.X), dt);
	for (ofloat &y : px) y = _mm256_mul_ps(_mm256_mul_ps(y, g.Y), dt);
	for (ofloat &z : px) z = _mm256_mul_ps(_mm256_mul_ps(z, g.Z), dt);
}

void Pu::MovementSystem::ApplyDrag(ofloat dt)
{
	for (size_t i = 0; i < vx.sse_size(); i++)
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
		const ofloat l = _mm256_sqrt_ps(ll);
		const ofloat ld = _mm256_mul_ps(ll, cod[i]);

		/* Calculate the aerodynamic force to apply. */
		const ofloat fx = _mm256_mul_ps(_mm256_div_ps(x, l), ld);
		const ofloat fy = _mm256_mul_ps(_mm256_div_ps(y, l), ld);
		const ofloat fz = _mm256_mul_ps(_mm256_div_ps(z, l), ld);

		/* Apply the force scaled with delta time. */
		x = _mm256_sub_ps(x, _mm256_mul_ps(fx, _mm256_mul_ps(m[i], dt)));
		y = _mm256_sub_ps(y, _mm256_mul_ps(fy, _mm256_mul_ps(m[i], dt)));
		z = _mm256_sub_ps(z, _mm256_mul_ps(fz, _mm256_mul_ps(m[i], dt)));
	}
}

void Pu::MovementSystem::Integrate(ofloat dt)
{
	/* Add linear velocity to position (scaled by delta time). */
	for (size_t i = 0; i < vx.sse_size(); i++) px[i] = _mm256_add_ps(px[i], _mm256_mul_ps(vx[i], dt));
	for (size_t i = 0; i < vy.sse_size(); i++) py[i] = _mm256_add_ps(py[i], _mm256_mul_ps(vy[i], dt));
	for (size_t i = 0; i < vz.sse_size(); i++) pz[i] = _mm256_add_ps(pz[i], _mm256_mul_ps(vz[i], dt));

	/* Add angular rotation to orientation (scaled by delta time). */
	for (size_t i = 0; i < wi.sse_size(); i++) ti[i] = _mm256_add_ps(ti[i], _mm256_mul_ps(wi[i], dt));
	for (size_t i = 0; i < wj.sse_size(); i++) ti[i] = _mm256_add_ps(tj[i], _mm256_mul_ps(wj[i], dt));
	for (size_t i = 0; i < wk.sse_size(); i++) ti[i] = _mm256_add_ps(tk[i], _mm256_mul_ps(wk[i], dt));
	for (size_t i = 0; i < wr.sse_size(); i++) ti[i] = _mm256_add_ps(tr[i], _mm256_mul_ps(wr[i], dt));
}

Pu::Matrix Pu::MovementSystem::CreateTransform(size_t idx) const
{
	const Matrix translation = Matrix::CreateTranslation(px.get(idx), py.get(idx), pz.get(idx));
	const Matrix orientation = Matrix::CreateRotation(Quaternion(tr.get(idx), ti.get(idx), tj.get(idx), tk.get(idx)));
	return translation * orientation;
}