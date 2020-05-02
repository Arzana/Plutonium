#pragma once
#include "FpsCamera.h"
#include "Input/InputDeviceHandler.h"

namespace Pu
{
	/* Defines a camera which follows a target with a specified offset. */
	class FollowCamera
		: public FpsCamera
	{
	public:
		/* Defines the desired spherical offset from the camera to the target. */
		float Distance;
		/* The movement speed modifier of the camera. */
		float MoveSpeed;
		/* The viewing speed modifier of the camera. */
		float LookSpeed;
		/* Defines the minimum damping factor for movement changes. */
		float MinDamping;
		/* Defines the maximum damping factor for movement changes. */
		float MaxDamping;
		/* Defines whether the camera should rotate towards the target. */
		bool LookAt;
		/* Whether the mouse pitch control should be inverted. */
		bool Inverted;

		/* Initializes a new instance of a follow camera. */
		FollowCamera(_In_ const NativeWindow &wnd, _In_ DescriptorPool &pool, _In_ const Renderpass &renderpass, _In_ const InputDeviceHandler &inputHandler);
		FollowCamera(_In_ const FollowCamera&) = delete;
		/* Move constructor. */
		FollowCamera(_In_ FollowCamera &&value);
		/* Releases the resources allocated by the follow camera. */
		virtual ~FollowCamera(void)
		{
			Destroy();
		}

		_Check_return_ FollowCamera& operator =(_In_ const FollowCamera&) = delete;
		/* Copy assignment. */
		_Check_return_ FollowCamera& operator =(_In_ FollowCamera &&other);

		/* Updates the follow camera. */
		void Update(_In_ float dt);

		/* Sets the follow target for the camera. */
		inline void SetTarget(_In_ const Matrix &value)
		{
			target = &value;
		}

	private:
		const Matrix *target;
		Vector2 lookDelta;
		float yaw, pitch;
		const InputDeviceHandler *inputHandler;

		void MouseMovedEventHandler(const Mouse&, Vector2 delta);
		void AddCallback(void);
		void Destroy(void);
	};
}