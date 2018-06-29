#pragma once
#include "Input\Keyboard.h"
#include "Input\Cursor.h"
#include "Core\Math\Matrix.h"
#include "Graphics\Native\Window.h"

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

		/* Initializes a new instance of a camera. */
		Camera(_In_ WindowHandler wnd);
		/* Releases the resources allocated by teh camera. */
		~Camera(void);

		/* Updates the camera as a 3rd person follow camera. */
		void Update(_In_ float dt, _In_ const Matrix &obj2Follow);
		/* Updates the camera as a free camera. */
		void Update(_In_ float dt, _In_ KeyHandler keys, _In_ CursorHandler cursor);
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
		/* Gets the current projection matrix. */
		_Check_return_ inline const Matrix& GetProjection(void) const
		{
			return proj;
		}

		/* Gets the position of the camera. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return actualPos;
		}

	private:
		Vector3 actualPos, desiredPos;
		Vector3 target, offset;
		Matrix view, proj, orien;
		WindowHandler wnd;

		void UpdatePosition(float dt);
		void UpdateView(void);
		void WindowResizeEventHandler(WindowHandler sender, EventArgs args);
	};
}