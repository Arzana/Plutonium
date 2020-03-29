#pragma once
#include "Camera.h"
#include "Core/Events/ValueChangedEventArgs.h"

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
		FpsCamera(_In_ const NativeWindow &wnd, _In_ DescriptorPool &pool, _In_ const Renderpass &renderpass);
		FpsCamera(_In_ const FpsCamera&) = delete;
		/* Move constructor. */
		FpsCamera(_In_ FpsCamera &&value) = default;

		_Check_return_ FpsCamera& operator =(_In_ const FpsCamera&) = delete;
		/* Move assignment. */
		_Check_return_ FpsCamera& operator =(_In_ FpsCamera &&other) = default;

		/* Sets the near clipping plane. */
		void SetNear(_In_ float value);
		/* Sets the far clipping plane. */
		void SetFar(_In_ float value);
		/* Sets the fied of view (vertical in radians). */
		void SetFoV(_In_ float value);
		/* Updates the FPS camera. */
		virtual void Update(_In_ float dt) override;

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

		/* Gets the orientation of the camera. */
		_Check_return_ inline const Matrix& GetOrientation(void) const
		{
			return orien;
		}

	protected:
		/* Occurs when the native window changes it's size. */
		virtual void OnWindowResize(const NativeWindow &sender, ValueChangedEventArgs<Vector2> args);

	private:
		float near, far, fov, aspr;
		Matrix orien;

		void UpdateProjection(void);
	};
}