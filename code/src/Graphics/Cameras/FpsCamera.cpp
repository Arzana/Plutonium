#include "Graphics/Cameras/FpsCamera.h"
#include "Application.h"

Pu::FpsCamera::FpsCamera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass)
	: Camera(wnd, pool, renderpass), near(0.1f), far(1000.0f), fov(PI4), aspr(wnd.GetAspectRatio())
{
	UpdateProjection();
}

void Pu::FpsCamera::SetNear(float value)
{
	near = value;
	UpdateProjection();
}

void Pu::FpsCamera::SetFar(float value)
{
	far = value;
	UpdateProjection();
}

void Pu::FpsCamera::SetFoV(float value)
{
	fov = value;
	UpdateProjection();
}

void Pu::FpsCamera::OnWindowResize(const NativeWindow & sender, ValueChangedEventArgs<Vector2> args)
{
	Camera::OnWindowResize(sender, args);

	aspr = sender.GetAspectRatio();
	UpdateProjection();
}

void Pu::FpsCamera::UpdateProjection(void)
{
	SetProjection(Matrix::CreatePerspective(fov, aspr, near, far));
}