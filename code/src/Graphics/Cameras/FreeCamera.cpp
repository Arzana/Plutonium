#include "Graphics/Cameras/FreeCamera.h"
#include "Input/Keys.h"
#include "Core/EnumUtils.h"
#include "Application.h"
#include <imgui/include/imgui.h>

/*
The state of the camera currently hold 5 values.
[FBLRM000]
F (bit 1): Whether the forward key is down.
B (bit 2): Whether the backward key is down.
L (bit 3): whether the left key is down.
R (bit 4): whether the right key is down.
M (bit 5): Whether the look delta need to be reset.

The movement states (F, B, L and R) are to make sure that repeat keys don't cause the camera to speed up.
The mouse state (M) is to make sure that we reset the look delta after a mouse input event,
this is needed because (unlike the slider input) the mouse doesn't reset itself.
*/
Pu::FreeCamera::FreeCamera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass, const InputDeviceHandler & inputHandler)
	: FpsCamera(wnd, pool, renderpass), state(0), inputHandler(&inputHandler),
	keyFrwd(_CrtEnum2Int(Keys::W)), keyBkwd(_CrtEnum2Int(Keys::S)),
	keyLeft(_CrtEnum2Int(Keys::A)), keyRight(_CrtEnum2Int(Keys::D)),
	MoveSpeed(1.0f), LookSpeed(6.0f), Inverted(false), DeadZone(0.05f)
{
	SetCallbacks();
}

Pu::FreeCamera::FreeCamera(FreeCamera && value)
	: FpsCamera(std::move(value)), inputHandler(value.inputHandler), state(value.state),
	keyFrwd(value.keyFrwd), keyBkwd(value.keyBkwd), keyLeft(value.keyLeft), keyRight(value.keyRight),
	moveDelta(value.moveDelta), lookDelta(value.lookDelta)
{
	SetCallbacks();
}

Pu::FreeCamera & Pu::FreeCamera::operator=(FreeCamera && other)
{
	if (this != &other)
	{
		Destroy();
		FpsCamera::operator=(std::move(other));

		inputHandler = other.inputHandler;
		state = other.state;
		keyFrwd = other.keyFrwd;
		keyBkwd = other.keyBkwd;
		keyLeft = other.keyLeft;
		keyRight = other.keyRight;
		moveDelta = other.moveDelta;
		lookDelta = other.lookDelta;

		SetCallbacks();
	}

	return *this;
}

void Pu::FreeCamera::Update(float dt)
{
	/* The slider input might have come from a gamepad, so remove the deadzone. */
	if (moveDelta.LengthSquared() < DeadZone) moveDelta = 0.0f;
	if (lookDelta.LengthSquared() < DeadZone) lookDelta = 0.0f;

	/* Update the position and orientaion. */
	Move(GetOrientation() * moveDelta * MoveSpeed * dt);
	Yaw -= lookDelta.X * DEG2RAD * LookSpeed * dt;
	Pitch += lookDelta.Y * DEG2RAD * LookSpeed * dt;

	/* Reset the lookstate if needed. */
	lookDelta *= !(state & 16);
	state &= ~16;

	/* Update the orientation and the view matrix. */
	Orientation = Quaternion::Create(Pitch, Yaw, Roll);
	SetView(Matrix::CreateLookIn(GetPosition(), Orientation * Vector3::Forward(), Orientation * Vector3::Up()));
}

void Pu::FreeCamera::VisualizeInternal(void)
{
	FpsCamera::VisualizeInternal();
	ImGui::Checkbox("Invert Y", &Inverted);
}

void Pu::FreeCamera::KeyDownEventHandler(const InputDevice &, const ButtonEventArgs & args)
{
	moveDelta.Z += args.KeyCode == keyFrwd && !(state & 1);
	moveDelta.Z -= args.KeyCode == keyBkwd && !(state & 2);
	moveDelta.X -= args.KeyCode == keyLeft && !(state & 4);
	moveDelta.X += args.KeyCode == keyRight && !(state & 8);

	state |= args.KeyCode == keyFrwd;
	state |= (args.KeyCode == keyBkwd) << 1;
	state |= (args.KeyCode == keyLeft) << 2;
	state |= (args.KeyCode == keyRight) << 3;
}

void Pu::FreeCamera::KeyUpEventHandler(const InputDevice &, const ButtonEventArgs & args)
{
	moveDelta.Z -= args.KeyCode == keyFrwd;
	moveDelta.Z += args.KeyCode == keyBkwd;
	moveDelta.X += args.KeyCode == keyLeft;
	moveDelta.X -= args.KeyCode == keyRight;

	state &= ~static_cast<int>(args.KeyCode == keyFrwd);
	state &= ~(static_cast<int>(args.KeyCode == keyBkwd) << 1);
	state &= ~(static_cast<int>(args.KeyCode == keyLeft) << 2);
	state &= ~(static_cast<int>(args.KeyCode == keyRight) << 3);
}

void Pu::FreeCamera::MouseMovedEventHandler(const Mouse &, Vector2 delta)
{
	/* We need to mark the the next update for reset because this was a mouse event. */
	lookDelta.X -= delta.X;
	lookDelta.Y += BoolToScalar(Inverted) * delta.Y;
	state |= 16;
}

void Pu::FreeCamera::ValueEventHandler(const InputDevice &, const ValueEventArgs & args)
{
	if (args.Information.GetUsagePage() == HIDUsagePage::GenericDesktop)
	{
		const HIDUsageGenericDesktop usage = _CrtInt2Enum<HIDUsageGenericDesktop>(args.Information.GetUsageStart());

		switch (usage)
		{
		case (HIDUsageGenericDesktop::X):
			moveDelta.X = args.Value * 2.0f - 1.0f;
			break;
		case (HIDUsageGenericDesktop::Y):
			moveDelta.Z = -args.Value * 2.0f + 1.0f;
			break;
		case (HIDUsageGenericDesktop::Rx):
			lookDelta.X = -args.Value * 2.0f + 1.0f;
			break;
		case (HIDUsageGenericDesktop::Ry):
			lookDelta.Y = BoolToScalar(Inverted) * (args.Value * 2.0f - 1.0f);
			break;
		}
	}
}

void Pu::FreeCamera::SetCallbacks(void)
{
	inputHandler->AnyKeyDown.Add(*this, &FreeCamera::KeyDownEventHandler);
	inputHandler->AnyKeyUp.Add(*this, &FreeCamera::KeyUpEventHandler);
	inputHandler->AnyMouseMoved.Add(*this, &FreeCamera::MouseMovedEventHandler);
	inputHandler->AnyValueChanged.Add(*this, &FreeCamera::ValueEventHandler);
}

void Pu::FreeCamera::Destroy(void)
{
	inputHandler->AnyKeyDown.Remove(*this, &FreeCamera::KeyDownEventHandler);
	inputHandler->AnyKeyUp.Remove(*this, &FreeCamera::KeyUpEventHandler);
	inputHandler->AnyMouseMoved.Remove(*this, &FreeCamera::MouseMovedEventHandler);
	inputHandler->AnyValueChanged.Remove(*this, &FreeCamera::ValueEventHandler);
}