#include "Components\Camera.h"
#include "Core\Math\Interpolation.h"

Plutonium::Camera::Camera(WindowHandler wnd)
	: wnd(wnd), actualPos(0.0f, 0.0f, 50.0f), 
	target(), offset(0.0f, 0.0f, 0.5f),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f), 
	MoveSpeed(100.0f), LookSpeed(0.1f),
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
	desiredPos = obj2Follow * offset;

	/* Update position and view matrix. */
	UpdatePosition();
	UpdateView();
}

void Plutonium::Camera::Update(float dt, KeyHandler keys, CursorHandler cursor)
{
	/* Update orientation. */
	Yaw -= cursor->DeltaX * DEG2RAD * LookSpeed;
	Pitch -= cursor->DeltaY * DEG2RAD * LookSpeed;

	/* Update desired position. */
	if (keys->IsKeyDown(Keys::W)) desiredPos += orien.GetForward() * dt * MoveSpeed;
	if (keys->IsKeyDown(Keys::A)) desiredPos += orien.GetLeft() * dt * MoveSpeed;
	if (keys->IsKeyDown(Keys::S)) desiredPos += orien.GetBackward() * dt * MoveSpeed;
	if (keys->IsKeyDown(Keys::D)) desiredPos += orien.GetRight() * dt * MoveSpeed;

	/* Update position and target. */
	UpdatePosition();
	target = actualPos + orien.GetForward();

	UpdateView();
}

void Plutonium::Camera::UpdatePosition(void)
{
	actualPos.X = smoothstep(actualPos.X, desiredPos.X, 0.15f);
	actualPos.Y = smoothstep(actualPos.Y, desiredPos.Y, 0.15f);
	actualPos.Z = smoothstep(actualPos.Z, desiredPos.Z, 0.15f);
}

void Plutonium::Camera::UpdateView(void)
{
	/* Update orientation. */
	orien.SetOrientation(Yaw, Pitch, Roll);

	/* Update view. */
	view = Matrix::CreateLookAt(actualPos, target, orien.GetUp());
}

void Plutonium::Camera::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	proj = Matrix::CreatPerspective(PI4, sender->AspectRatio(), 0.1f, 100.0f);
}
