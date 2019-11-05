#include "Components/FreeCamera.h"
#include "Input/Keys.h"
#include "Core/EnumUtils.h"
#include "Application.h"

Pu::FreeCamera::FreeCamera(Application & app, const InputDeviceHandler & inputHandler)
	: FpsCamera(app), keyStates(0), inputHandler(inputHandler),
	keyFrwd(_CrtEnum2Int(Keys::W)), keyBkwd(_CrtEnum2Int(Keys::S)),
	keyLeft(_CrtEnum2Int(Keys::A)), keyRight(_CrtEnum2Int(Keys::D)),
	MoveSpeed(1.0f), LookSpeed(6.0f), Inverted(false)
{
	inputHandler.AnyKeyDown.Add(*this, &FreeCamera::KeyDownEventHandler);
	inputHandler.AnyKeyUp.Add(*this, &FreeCamera::KeyUpEventHandler);
	inputHandler.AnyMouseMoved.Add(*this, &FreeCamera::MouseMovedEventHandler);
}

void Pu::FreeCamera::Update(float dt)
{
	if (IsEnabled())
	{
		/* Update the position. */
		if (keyStates)
		{
			Vector3 moveDelta;

			if (keyStates & 1) moveDelta += GetOrientation().GetForward();
			if (keyStates & 2) moveDelta += GetOrientation().GetBackward();
			if (keyStates & 4) moveDelta += GetOrientation().GetLeft();
			if (keyStates & 8) moveDelta += GetOrientation().GetRight();

			Move(moveDelta * MoveSpeed * dt);
		}

		/* Update the orientaion. */
		Yaw -= lookDelta.X * DEG2RAD * LookSpeed * dt;
		Pitch -= lookDelta.Y * DEG2RAD * LookSpeed * dt;
	}

	lookDelta = Vector2();
	FpsCamera::Update(dt);
}

void Pu::FreeCamera::Finalize(void)
{
	FpsCamera::Finalize();

	inputHandler.AnyKeyDown.Remove(*this, &FreeCamera::KeyDownEventHandler);
	inputHandler.AnyKeyUp.Remove(*this, &FreeCamera::KeyUpEventHandler);
	inputHandler.AnyMouseMoved.Remove(*this, &FreeCamera::MouseMovedEventHandler);
}

void Pu::FreeCamera::KeyDownEventHandler(const InputDevice &, const ButtonEventArgs & args)
{
	if (args.KeyCode == keyFrwd) keyStates |= 1;
	else if (args.KeyCode == keyBkwd) keyStates |= 2;
	else if (args.KeyCode == keyLeft) keyStates |= 4;
	else if (args.KeyCode == keyRight) keyStates |= 8;
}

void Pu::FreeCamera::KeyUpEventHandler(const InputDevice &, const ButtonEventArgs & args)
{
	if (args.KeyCode == keyFrwd) keyStates &= ~1;
	else if (args.KeyCode == keyBkwd) keyStates &= ~2;
	else if (args.KeyCode == keyLeft) keyStates &= ~4;
	else if (args.KeyCode == keyRight) keyStates &= ~8;
}

void Pu::FreeCamera::MouseMovedEventHandler(const Mouse &, Vector2 delta)
{
	lookDelta.X -= delta.X;
	lookDelta.Y -= Inverted ? -delta.Y : delta.Y;
}