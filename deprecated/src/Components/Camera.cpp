#include "Components\Camera.h"
#include "Core\Math\VInterpolation.h"

Plutonium::Camera::Camera(GraphicsAdapter * device)
	: device(device), actualPos(), 
	target(), Offset(0.0f, 0.0f, 0.5f),
	Yaw(0.0f), Pitch(0.0f), Roll(0.0f), 
	MoveSpeed(100.0f), LookSpeed(6.0f),
	near(0.1f), far(1000.0f), fov(PI4), 
	orien(), iviewDirty(true)
{
	desiredPos = actualPos;
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
	device->GetWindow()->SizeChanged.Add(this, &Camera::WindowResizeEventHandler);
}

Plutonium::Camera::~Camera(void)
{
	device->GetWindow()->SizeChanged.Remove(this, &Camera::WindowResizeEventHandler);
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

Plutonium::Vector3 Plutonium::Camera::ScreenToWorldRay(Vector2 v) const
{
	/* Convert the screen coordinate to a normalized device coordinate. */
	Vector2 ndc = device->ToNDC(v);
	
	/* Convert the ndc to clip space, z needs to be set to -1 to indicate the near plane and w needs to be set to 1 to indicate no change at the perspective divide. */
	Vector4 clip(ndc.X, ndc.Y, -1.0f, 1.0f);

	/* Convert to clip space coordinate back to view space. */
	Vector4 eye = GetInverseProjection() * clip;
	eye = Vector4(eye.X, eye.Y, -1.0f, 1.0f);

	/* Converts the view space coordinate back to world space and return the normalized 3D part. */
	Vector4 world = GetInverseView() * eye;
	return normalize(world.XYZ - actualPos);
}

Plutonium::Vector3 Plutonium::Camera::ScreenToWorld(Vector2 v, float z) const
{
	/* Convert the screen coordinate to a normalized device coordinate. */
	Vector2 ndc = device->ToNDC(v);

	/* Convert the ndc to clip space, the z component needs to be bound to the ndc space as well. */
	Vector4 clip(ndc.X, ndc.Y, ilerp(GetNear(), GetFar(), z) - 1.0f, 1.0f);

	/* Convert the clip space coordinate back to view space. */
	Vector4 eye = GetInverseProjection() * clip;

	/* Perform perspective division. */
	eye /= eye.W;

	/* The w component can be discarded because no world space coordinate would have one. */
	return (GetInverseView() * eye).XYZ;
}

void Plutonium::Camera::SetNear(float value)
{
	if (value == near) return;
	near = value;
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
}

void Plutonium::Camera::SetFar(float value)
{
	if (value == far) return;
	far = value;
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
}

void Plutonium::Camera::SetFoV(float value)
{
	if (value == fov) return;
	fov = value;
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
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
	view = Matrix::CreateLookAt(target, actualPos, orien.GetUp());
	frustum = Frustum(proj * view);
	iviewDirty = true;
}

void Plutonium::Camera::WindowResizeEventHandler(WindowHandler sender, EventArgs)
{
	proj = Matrix::CreatPerspective(fov, sender->AspectRatio(), near, far);
	iproj = proj.GetInverse();
}