#pragma once
#include "FpsCamera.h"
#include "Input/InputDeviceHandler.h"

namespace Pu
{
	/* Defines a camera that can be moved using keyboard and cursor input. */
	class FreeCamera
		: public FpsCamera
	{
	public:
		/* The yaw that the camera should have. */
		float Yaw;
		/* The pitch that the camera should have. */
		float Pitch;
		/* Rhe roll that the camera should have. */
		float Roll;
		/* The movement speed modifier of the camera. */
		float MoveSpeed;
		/* The viewing speed modifier of the camera. */
		float LookSpeed;
		/* Defines the dead zone of the slider input. */
		float DeadZone;
		/* Whether the mouse pitch control should be inverted. */
		bool Inverted;

		/* 
		Initializes a new instance of a free camera.
		Default controlls are WASD for movement and normal cursor controlls.
		The keyboard to take input from can only be specified here.
		*/
		FreeCamera(_In_ const NativeWindow &wnd, _In_ DescriptorPool &pool, _In_ const Renderpass &renderpass, _In_ const InputDeviceHandler &inputHandler);
		FreeCamera(_In_ const FreeCamera&) = delete;
		/* Move constructor. */
		FreeCamera(_In_ FreeCamera &&value);
		/* Releases the resources allocated by the free camera. */
		virtual ~FreeCamera(void)
		{
			Destroy();
		}

		_Check_return_ FreeCamera& operator =(_In_ const FreeCamera&) = delete;
		/* Move assignment. */
		_Check_return_ FreeCamera& operator =(_In_ FreeCamera &&other);

		/* Updates the free camera. */
		void Update(_In_ float dt);

		/* Sets the key used to move the camera forward. */
		inline void SetForwardKey(_In_ uint16 key)
		{
			keyFrwd = key;
		}

		/* Sets the key used to move the camera backward. */
		inline void SetBackwardKey(_In_ uint16 key)
		{
			keyBkwd = key;
		}

		/* Sets the key used to move the camera to the left. */
		inline void SetLeftKey(_In_ uint16 key)
		{
			keyLeft = key;
		}

		/* Sets the key used to move the camera to the right. */
		inline void SetRightKey(_In_ uint16 key)
		{
			keyRight = key;
		}

	protected:
		/* Occurs when the user visualizes the camera. */
		virtual void VisualizeInternal(void);

	private:
		uint16 keyFrwd, keyBkwd, keyLeft, keyRight;
		byte state;
		Vector3 moveDelta;
		Vector2 lookDelta;
		const InputDeviceHandler *inputHandler;

		void KeyDownEventHandler(const InputDevice&, const ButtonEventArgs &args);
		void KeyUpEventHandler(const InputDevice&, const ButtonEventArgs &args);
		void MouseMovedEventHandler(const Mouse&, Vector2 delta);
		void ValueEventHandler(const InputDevice&, const ValueEventArgs &args);
		void SetCallbacks(void);
		void Destroy(void);
	};
}