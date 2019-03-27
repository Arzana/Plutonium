#pragma once
#include "FpsCamera.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"

namespace Pu
{
	/* Defines a camera that can be moved using keyboard and cursor input. */
	class FreeCamera
		: public FpsCamera
	{
	public:
		/* The movement speed modifier of the camera. */
		float MoveSpeed;
		/* The viewing speed modifier of the camera. */
		float LookSpeed;
		/* Whether the cursor pitch control should be inverted. */
		bool Inverted;

		/* 
		Initializes a new instance of a free camera.
		Default controlls are WASD for movement and normal cursor controlls.
		The keyboard to take input from can only be specified here.
		*/
		FreeCamera(_In_ Application &app, _In_ const Keyboard &keyboard, _In_ const Mouse &cursor);
		FreeCamera(_In_ const FreeCamera&) = delete;
		FreeCamera(_In_ FreeCamera&&) = delete;

		_Check_return_ FreeCamera& operator =(_In_ const FreeCamera&) = delete;
		_Check_return_ FreeCamera& operator =(_In_ FreeCamera&&) = delete;

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
		/* pdates the camera's position and orientation. */
		virtual void Update(_In_ float dt) override;
		/* Finalizes the free camera. */
		virtual void Finalize(void) override;

	private:
		uint16 keyFrwd, keyBkwd, keyLeft, keyRight;
		byte keyStates;
		const Keyboard &keyboard;
		const Mouse &cursor;
		Vector2 lookDelta;

		void KeyDownEventHandler(const Keyboard&, uint16 key);
		void KeyUpEventHandler(const Keyboard&, uint16 key);
		void MouseMovedEventHandler(const Mouse&, Vector2 delta);
	};
}