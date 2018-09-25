#pragma once
#include "Input\Keyboard.h"
#include "Input\Cursor.h"
#include "Core\Math\Frustum.h"
#include "Graphics\GraphicsAdapter.h"

#if defined(far)
#undef far
#endif

#if defined(near)
#undef near
#endif

namespace Plutonium
{
	/* Defines a basic follow and free camera. */
	struct Camera
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
		/* The offset the camera will have from the viewing target. */
		Vector3 Offset;

		/* Initializes a new instance of a camera. */
		Camera(_In_ GraphicsAdapter * device);
		/* Releases the resources allocated by teh camera. */
		~Camera(void);

		/* Updates the camera as a 3rd person follow camera. */
		void Update(_In_ float dt, _In_ const Matrix &obj2Follow);
		/* Updates the camera as a free camera. */
		void Update(_In_ float dt, _In_ KeyHandler keys, _In_ CursorHandler cursor);
		/* Converts the specified screen coordinate to world space. */
		_Check_return_ Vector3 ScreenToWorldRay(_In_ Vector2 v) const;
		/* Converts the specified screen coordinate to word space. */
		_Check_return_ Vector3 ScreenToWorld(_In_ Vector2 v, _In_ float z) const;
		/* Teleports the camera to a specified position. */
		inline void Move(_In_ Vector3 position)
		{
			desiredPos = position;
		}

		/* Gets the current view matrix. */
		_Check_return_ inline const Matrix& GetView(void) const
		{
			return view;
		}

		/* Gets the current inverse view matrix. */
		_Check_return_ inline const Matrix& GetInverseView(void) const
		{
			if (iviewDirty)
			{
				iviewDirty = false;
				iview = view.GetInverse();
			}

			return iview;
		}

		/* Gets the current projection matrix. */
		_Check_return_ inline const Matrix& GetProjection(void) const
		{
			return proj;
		}

		/* Gets the current inverse projection matrix. */
		_Check_return_ inline const Matrix& GetInverseProjection(void) const
		{
			return iproj;
		}

		/* Gets the current orientation matrix. */
		_Check_return_ inline const Matrix& GetOrientation(void) const
		{
			return orien;
		}

		/* Gets the position of the camera. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return actualPos;
		}

		/* Gets the current near clipping plane. */
		_Check_return_ inline float GetNear(void) const
		{
			return near;
		}

		/* Gets the current far clipping plane. */
		_Check_return_ inline float GetFar(void) const
		{
			return far;
		}

		/* Gets the current field of view. */
		_Check_return_ inline float GetFov(void) const
		{
			return fov;
		}

		/* Gets the frustum used for frustum culling. */
		_Check_return_ inline const Frustum* GetClip(void) const
		{
			return &frustum;
		}

		/* Sets the near clipping plane. */
		void SetNear(_In_ float value);
		/* Sets the far clipping plane. */
		void SetFar(_In_ float value);
		/* Sets the field of view. */
		void SetFoV(_In_ float value);

	private:
		float near, far, fov;
		Vector3 actualPos, desiredPos;
		Vector3 target;
		Matrix view, proj, iproj, orien;
		Frustum frustum;
		GraphicsAdapter *device;

		mutable bool iviewDirty;
		mutable Matrix iview;

		void UpdatePosition(float dt);
		void UpdateView(void);
		void WindowResizeEventHandler(WindowHandler sender, EventArgs);
	};
}