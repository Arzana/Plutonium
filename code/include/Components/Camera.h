#pragma once
#include "Component.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a camera base. */
	class Camera
		: public Component
	{
	public:
		/* Copy constructor. */
		Camera(_In_ const Camera &value);
		/* Move constructor. */
		Camera(_In_ Camera &&value);

		_Check_return_ Camera& operator =(_In_ const Camera&) = delete;
		_Check_return_ Camera& operator =(_In_ Camera&&) = delete;

		/* Convertes the specified screen coordinate to a ray pointing from the camera's position into the world. */
		_Check_return_ Vector3 ScreenToWorldRay(_In_ Vector2 v) const;
		/* Converts the specified screen coordinate to a world space position (z is in NDC space). */
		_Check_return_ Vector3 ScreenToWorld(_In_ Vector2 v, _In_ float z) const;
		/* Gets the camera's inverse view matrix. */
		_Check_return_ const Matrix& GetInverseView(void) const;

		/* Gets the position of the camera. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return pos;
		}

		/* Gets the camera's view matrix. */
		_Check_return_ inline const Matrix& GetView(void) const
		{
			return view;
		}

		/* Gets the camera's projection matrix. */
		_Check_return_ inline const Matrix& GetProjection(void) const
		{
			return proj;
		}

		/* Gets the camera's inverse projection matrix. */
		_Check_return_ inline const Matrix& GetInverseProjection(void) const
		{
			return iproj;
		}

		/* Sets the position of the camera. */
		inline virtual void SetPosition(_In_ Vector3 position)
		{
			pos = position;
		}

		/* Adds the specified offset to the position. */
		inline void Move(_In_ Vector3 offset)
		{
			SetPosition(pos + offset);
		}

		/* Adds the specified offset to the position. */
		inline void Move(_In_ float x, _In_ float y, _In_ float z)
		{
			SetPosition(pos + Vector3(x, y, z));
		}

	protected:
		/* Initializes a new instance of a camera. */
		Camera(_In_ Application &app);

		/* Sets the view matrix. */
		void SetView(_In_ const Matrix &value);
		/* Sets the projection matrix. */
		void SetProjection(_In_ const Matrix &value);
		/* Updtes the camera. */
		virtual void Update(_In_ float dt) = 0;

	private:
		Vector3 pos;
		Matrix view, proj, iproj;

		mutable Matrix iview;
		mutable bool viewDirty;

		Vector2 ToNDC(Vector2 v) const;
	};
}