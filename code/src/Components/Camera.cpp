#include "Components/Camera.h"
#include "Application.h"

Pu::Camera::Camera(Application & app)
	: Component(app), viewDirty(false)
{}

Pu::Camera::Camera(const Camera & value)
	: Component(value), viewDirty(value.viewDirty),
	pos(value.pos), view(value.view), proj(value.proj),
	iproj(value.iproj), iview(value.iview)
{}

Pu::Camera::Camera(Camera && value)
	: Component(std::move(value)), viewDirty(value.viewDirty),
	pos(value.pos), view(value.view), proj(value.proj),
	iproj(value.iproj), iview(value.iview)
{}

Pu::Vector3 Pu::Camera::ScreenToWorldRay(Vector2 v) const
{
	/* Convert the screen coordinates to normalized device coordinates. */
	const Vector4 ndc = Vector4(ToNDC(v), 0.0f, 0.0f);

	/* Convert to eye space and perform perspective division. */
	Vector4 eye = iproj * ndc;
	eye /= eye.W;

	/* Convert to world space and return the normalized direction from the position on the near plane to the camera. */
	const Vector4 world = GetInverseView() * eye;
	return normalize(world.XYZ - pos);
}

Pu::Vector3 Pu::Camera::ScreenToWorld(Vector2 v, float z) const
{
	/* Convert from screen coordinates to normalized device coordinates. */
	const Vector4 ndc = Vector4(ToNDC(v), z, 1.0f);
	
	/* Convert to eye space and perform perspective division. */
	Vector4 eye = iproj * ndc;
	eye /= eye.W;

	/* The w component can be discarded because no world space coordinate would have one. */
	return (GetInverseView() * eye).XYZ;
}

const Pu::Matrix & Pu::Camera::GetInverseView(void) const
{
	if (viewDirty)
	{
		iview = view.GetInverse();
		viewDirty = false;
	}

	return iview;
}

void Pu::Camera::SetView(const Matrix & value)
{
	view = value;
	viewDirty = true;
}

void Pu::Camera::SetProjection(const Matrix & value)
{
	proj = value;
	iproj = proj.GetInverse();
}

Pu::Vector2 Pu::Camera::ToNDC(Vector2 v) const
{
	const Viewport vp = App.GetWindow().GetNative().GetClientBounds();
	return v / Vector2(vp.Width, vp.Height) * 2.0f - Vector2(1.0f);
}