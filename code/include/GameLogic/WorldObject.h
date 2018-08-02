#pragma once
#include "Core\Math\Matrix.h"
#include "Core\Math\Box.h"

namespace Plutonium
{
	/* Defines a base for object that can be placed in the game world. */
	struct WorldObject
	{
	public:
		/* Whether this object should cast a shadow. */
		bool CastsShadows;

		/* Gets a box defining the bounds of this object. */
		_Check_return_ inline Box GetBoundingBox(void) const
		{
			return bb * transform;
		}

		/* Gets the world position of this object. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return transform.GetTranslation();
		}

		/* Gets the world (or model matrix) of this object. */
		_Check_return_ inline const Matrix& GetWorld(void) const
		{
			return transform;
		}

		/* Sets the scale of the object. */
		virtual void SetScale(_In_ float scale)
		{
			transform.SetScale(scale);
		}

		/* Sets the orientation of the object. */
		virtual void SetOrientation(_In_ float yaw, _In_ float pitch, _In_ float roll)
		{
			transform.SetOrientation(yaw, pitch, roll);
		}

		/* Sets the orientation of the object. */
		inline virtual void SetOrientation(_In_ Vector3 orientation)
		{
			SetOrientation(orientation.X, orientation.Y, orientation.Z);
		}

		/* Teleports the object to a new position. */
		virtual void Teleport(_In_ Vector3 pos)
		{
			transform.SetTranslation(pos);
		}

		/* Moves the object by a specified offset. */
		virtual void Move(_In_ Vector3 amnt)
		{
			Teleport(GetPosition() + amnt);
		}

	protected:
		/* Initializes a new instance of a world object. */
		WorldObject(void)
			: transform(), bb(), CastsShadows(true)
		{}

		Matrix transform;
		Box bb;
	};
}