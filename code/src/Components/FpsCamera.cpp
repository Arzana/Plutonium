#include "Components/FpsCamera.h"
#include "Application.h"

Pu::FpsCamera::FpsCamera(Application & app)
	: Camera(app), projDirty(false),
	near(0.1f), far(1000.0f), fov(PI4),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f)
{
	App.GetWindow().GetNative().OnSizeChanged.Add(*this, &FpsCamera::WindowResizeEventHandler);
	WindowResizeEventHandler(App.GetWindow().GetNative(), ValueChangedEventArgs<Vector2>(Vector2(), Vector2()));
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

void Pu::FpsCamera::Update(float)
{
	/* Update the projection if needed. */
	if (projDirty) WindowResizeEventHandler(App.GetWindow().GetNative(), ValueChangedEventArgs<Vector2>(Vector2(), Vector2()));

	/* Update the camera's orientation. */
	orien.SetOrientation(Yaw, Pitch, Roll);

	/* Update the view matrix and the clipping frustum. */
	SetView(Matrix::CreateLookIn(GetPosition(), orien.GetForward(), orien.GetUp()));
	frustum = Frustum(GetProjection() * GetView());
}

void Pu::FpsCamera::Finalize(void)
{
	Camera::Finalize();
	App.GetWindow().GetNative().OnSizeChanged.Remove(*this, &FpsCamera::WindowResizeEventHandler);
}

void Pu::FpsCamera::WindowResizeEventHandler(const NativeWindow & sender, ValueChangedEventArgs<Vector2>)
{
	projDirty = false;
	SetProjection(Matrix::CreatPerspective(fov, sender.GetAspectRatio(), near, far));
}