#include "Graphics/Cameras/FollowCamera.h"
#include "Core/Math/Interpolation.h"

Pu::FollowCamera::FollowCamera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass, const InputDeviceHandler & inputHandler)
	: FpsCamera(wnd, pool, renderpass), Distance(5.0f), inputHandler(&inputHandler),
	MoveSpeed(1.0f), LookSpeed(6.0f), MinDamping(13.0f), MaxDamping(15.0f), 
	LookAt(true), Inverted(false), yaw(0.0f), pitch(0.0f)
{
	AddCallback();
}

Pu::FollowCamera::FollowCamera(FollowCamera && value)
	: FpsCamera(std::move(value)), inputHandler(value.inputHandler), target(value.target),
	lookDelta(value.lookDelta), Distance(value.Distance), MoveSpeed(value.MoveSpeed),
	LookSpeed(value.LookSpeed), MinDamping(value.MinDamping), MaxDamping(value.MaxDamping),
	LookAt(value.LookAt), Inverted(value.Inverted), yaw(value.yaw), pitch(value.pitch)
{
	AddCallback();
}

Pu::FollowCamera & Pu::FollowCamera::operator=(FollowCamera && other)
{
	if (this != &other)
	{
		Destroy();
		FpsCamera::operator=(std::move(other));

		inputHandler = other.inputHandler;
		target = other.target;
		lookDelta = other.lookDelta;
		Distance = other.Distance;
		MoveSpeed = other.MoveSpeed;
		LookSpeed = other.LookSpeed;
		MinDamping = other.MinDamping;
		MaxDamping = other.MaxDamping;
		LookAt = other.LookAt;
		Inverted = other.Inverted;
		yaw = other.yaw;
		pitch = other.pitch;

		AddCallback();
	}

	return *this;
}

void Pu::FollowCamera::Update(float dt)
{
	if (target)
	{
		const Vector3 translation = target->GetTranslation();
		yaw -= lookDelta.X * DEG2RAD * LookSpeed * dt;
		pitch += lookDelta.Y * DEG2RAD * LookSpeed * dt;
		lookDelta = 0.0f;

		/* Calculate the desired offset for the camera. */
		Orientation = Quaternion::Create(yaw, pitch, 0.0f);
		const Vector3 offset = Orientation * (Vector3::Backward() * Distance);

		/* Apply some damping to make the camera movement smoother. */
		const Vector3 desiredPosition = translation + offset;
		const float d = dist(GetPosition(), translation);
		SetPosition(damp(GetPosition(), desiredPosition, max(MinDamping, MaxDamping - d) * MoveSpeed, dt));

		/* Update the view matrix to always have the target in the center of the viewing plane. */
		if (LookAt) SetView(Matrix::CreateLookAt(GetPosition(), translation, Orientation * Vector3::Up()));
	}

	if (!LookAt)
	{
		/* We still need to update the view matrix incase the orientation has changed. */
		SetView(Matrix::CreateLookIn(GetPosition(), Orientation * Vector3::Forward(), Orientation * Vector3::Up()));
	}
}

void Pu::FollowCamera::MouseMovedEventHandler(const Mouse &, Vector2 delta)
{
	lookDelta.X -= delta.X;
	lookDelta.Y += BoolToScalar(Inverted) * delta.Y;
}

void Pu::FollowCamera::AddCallback(void)
{
	inputHandler->AnyMouseMoved.Add(*this, &FollowCamera::MouseMovedEventHandler);
}

void Pu::FollowCamera::Destroy(void)
{
	inputHandler->AnyMouseMoved.Remove(*this, &FollowCamera::MouseMovedEventHandler);
}