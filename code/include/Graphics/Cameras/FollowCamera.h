#pragma once
#include "FpsCamera.h"

namespace Pu
{
	/* Defines a camera which follows a target with a specified offset. */
	class FollowCamera
		: public FpsCamera
	{
	public:
		/* Defines the desired backwards horizontal offset from the camera to the target. */
		float Distance;
		/* Defines the desited upwards vertical offset from the camera to the target. */
		float Height;
		/* Defines the movement speed of the camera. */
		float Speed;
		/* Defines the minimum damping factor for movement changes. */
		float MinDamping;
		/* Defines the maximum damping factor for movement changes. */
		float MaxDamping;
		/* Defines whether the camera should rotate towards the target. */
		bool LookAt;

		/* Initializes a new instance of a follow camera. */
		FollowCamera(_In_ const NativeWindow &wnd, _In_ DescriptorPool &pool, _In_ const Renderpass &renderpass);
		FollowCamera(_In_ const FollowCamera&) = delete;
		/* Move constructor. */
		FollowCamera(_In_ FollowCamera &&value) = default;

		_Check_return_ FollowCamera& operator =(_In_ const FollowCamera&) = delete;
		/* Copy assignment. */
		_Check_return_ FollowCamera& operator =(_In_ FollowCamera &&other) = default;

		/* Updates the follow camera. */
		void Update(_In_ float dt);

		/* Sets the follow target for the camera. */
		inline void SetTarget(_In_ const Matrix &value)
		{
			target = &value;
		}

	private:
		const Matrix *target;
	};
}