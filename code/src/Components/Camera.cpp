#include "Components\Camera.h"
#include "Core\Math\Interpolation.h"

Plutonium::Camera::Camera(WindowHandler wnd)
	: wnd(wnd), actualPos(0.0f, 0.0f, 50.0f), 
	target(), offset(0.0f, 0.0f, 5.0f),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f), Speed(100.0f),
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
	/* Update orientation and target. */
	orien.SetOrientation(Yaw, Pitch, Roll);
	target = obj2Follow.GetTranslation();
	desiredPos = obj2Follow * offset;

	/* Update follow position. */
	actualPos.X = smoothstep(actualPos.X, desiredPos.X, 0.15f);
	actualPos.Y = smoothstep(actualPos.Y, desiredPos.Y, 0.15f);
	actualPos.Z = smoothstep(actualPos.Z, desiredPos.Z, 0.15f);

	/* Update view. */
	view = Matrix::CreateLookAt(actualPos, target, orien.GetUp());
}

void Plutonium::Camera::Update(float dt, KeyHandler keys)
{
}

void Plutonium::Camera::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	proj = Matrix::CreatPerspective(PI4, sender->AspectRatio(), 0.5f, 2000.0f);
}
