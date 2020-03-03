#include "Graphics/Cameras/FpsCamera.h"
#include "Application.h"

Pu::FpsCamera::FpsCamera(const NativeWindow & wnd, DescriptorPool & pool)
	: Camera(wnd, pool), near(0.1f), far(1000.0f), fov(PI4),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f), aspr(wnd.GetAspectRatio())
{
	UpdateProjection();
}

void Pu::FpsCamera::SetNear(float value)
{
	near = value;
	projDirty = true;
}

void Pu::FpsCamera::SetFar(float value)
{
	far = value;
	projDirty = true;
}

void Pu::FpsCamera::SetFoV(float value)
{
	fov = value;
	projDirty = true;
}

void Pu::FpsCamera::Update(float dt)
{
	/* Update the projection if needed. */
	if (projDirty) UpdateProjection();

	/* Update the camera's orientation. */
	orien.SetOrientation(Yaw, Pitch, Roll);

	/* Update the view matrix and the clipping frustum. */
	SetView(Matrix::CreateLookIn(GetPosition(), orien.GetForward(), orien.GetUp()));
	frustum = Frustum(GetProjection() * GetView());

	Camera::Update(dt);
}

void Pu::FpsCamera::OnWindowResize(const NativeWindow & sender, ValueChangedEventArgs<Vector2> args)
{
	Camera::OnWindowResize(sender, args);

	aspr = sender.GetAspectRatio();
	UpdateProjection();
}

void Pu::FpsCamera::UpdateProjection(void)
{
	projDirty = false;
	SetProjection(Matrix::CreatePerspective(fov, aspr, near, far));
}