#include "Components\Camera.h"
#include "Core\Math\VInterpolation.h"

Plutonium::Camera::Camera(WindowHandler wnd)
	: wnd(wnd), actualPos(0.0f, 0.0f, 50.0f), 
	target(), Offset(0.0f, 0.0f, 0.5f),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f), 
	MoveSpeed(100.0f), LookSpeed(6.0f),
	near(0.1f), far(1000.0f), fov(PI4), 
	orien()
{
	desiredPos = actualPos;
	WindowResizeEventHandler(wnd, EventArgs());
	wnd->SizeChanged.Add(this, &Camera::WindowResizeEventHandler);
}

Plutonium::Camera::~Camera(void)
{
	wnd->SizeChanged.Remove(this, &Camera::WindowResizeEventHandler);
}

void Plutonium::Camera::Update(float dt, const Matrix & obj2Follow)
{
	/* Update target and desired position. */
	target = obj2Follow.GetTranslation();
	desiredPos = obj2Follow * Offset;

	/* Update position and view matrix. */
	UpdatePosition(dt);
	UpdateView();
}

void Plutonium::Camera::Update(float dt, KeyHandler keys, CursorHandler cursor)
{
	/* Update orientation. */
	if (cursor)
	{
		const float mod = DEG2RAD * dt * LookSpeed;
		Yaw -= cursor->DeltaX * mod;
		Pitch -= cursor->DeltaY * mod;
	}

	/* Update desired position. */
	if (keys)
	{
		if (keys->IsKeyDown(Keys::W)) desiredPos += orien.GetForward() * dt * MoveSpeed;
		if (keys->IsKeyDown(Keys::A)) desiredPos += orien.GetLeft() * dt * MoveSpeed;
		if (keys->IsKeyDown(Keys::S)) desiredPos += orien.GetBackward() * dt * MoveSpeed;
		if (keys->IsKeyDown(Keys::D)) desiredPos += orien.GetRight() * dt * MoveSpeed;
	}

	/* Update position and target. */
	UpdatePosition(dt);
	target = actualPos + orien.GetForward();

	UpdateView();
}

void Plutonium::Camera::SetNear(float value)
{
	if (value == near) return;
	near = value;
	WindowResizeEventHandler(wnd, EventArgs());
}

void Plutonium::Camera::SetFar(float value)
{
	if (value == far) return;
	far = value;
	WindowResizeEventHandler(wnd, EventArgs());
}

void Plutonium::Camera::SetFoV(float value)
{
	if (value == fov) return;
	fov = value;
	WindowResizeEventHandler(wnd, EventArgs());
}

void Plutonium::Camera::UpdatePosition(float dt)
{
	constexpr float SPEED = 6.0f;
	actualPos = interp(actualPos, desiredPos, dt, SPEED);
}

void Plutonium::Camera::UpdateView(void)
{
	/* Update orientation. */
	orien.SetOrientation(Yaw, Pitch, Roll);

	/* Update view. */
	view = Matrix::CreateLookAt(actualPos, target, orien.GetUp());
	frustum = Frustum(proj * view);
}

void Plutonium::Camera::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	proj = Matrix::CreatPerspective(fov, sender->AspectRatio(), near, far);
}