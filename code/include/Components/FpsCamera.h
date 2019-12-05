#pragma once
#include "Camera.h"
#include "Core/Math/Frustum.h"
#include "Core/Events/ValueChangedEventArgs.h"

#ifdef far
#undef far
#endif

#ifdef near
#undef near
#endif

namespace Pu
{
	class NativeWindow;

	/* Defines a perspective camera. */
	class FpsCamera
		: public Camera
	{
	public:
		/* The yaw that the camera should have. */
		float Yaw;
		/* The pitch that the camera should have. */
		float Pitch;
		/* Rhe roll that the camera should have. */
		float Roll;

		/* Initializes a new instance of a first person camera. */
		FpsCamera(_In_ Application &app);
		/* Copy constructor. */
		FpsCamera(_In_ const FpsCamera &value);
		/* Move constructor. */
		FpsCamera(_In_ FpsCamera &&value);

		_Check_return_ FpsCamera& operator =(_In_ const FpsCamera&) = delete;
		_Check_return_ FpsCamera& operator =(_In_ FpsCamera&&) = delete;

		/* Sets the near clipping plane. */
		void SetNear(_In_ float value);
		/* Sets the far clipping plane. */
		void SetFar(_In_ float value);
		/* Sets the fied of view (vertical in radians). */
		void SetFoV(_In_ float value);

		/* Gets the near clipping plane. */
		_Check_return_ inline float GetNear(void) const
		{
			return near;
		}

		/* Gets the far clipping plane. */
		_Check_return_ inline float GetFar(void) const
		{
			return far;
		}

		/* Gets the vertical field of view (in radians). */
		_Check_return_ inline float GetFoV(void) const
		{
			return fov;
		}

		/* Gets a frustum that can be used for frustum culling. */
		_Check_return_ inline const Frustum& GetClip(void) const
		{
			return frustum;
		}

		/* Gets the orientation of the camera. */
		_Check_return_ inline const Matrix& GetOrientation(void) const
		{
			return orien;
		}

	protected:
		/* Updates the camera's view. */
		virtual void Update(_In_ float dt) override;
		/* Finalizes the fps camera. */
		virtual void Finalize(void) override;

	private:
		float near, far, fov;
		Matrix orien;
		bool projDirty;
		Frustum frustum;

		void WindowResizeEventHandler(const NativeWindow &sender, ValueChangedEventArgs<Vector2>);
	};
}