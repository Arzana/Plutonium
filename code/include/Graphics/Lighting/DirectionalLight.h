#pragma once
#include "Light.h"
#include "Core\Math\Box.h"

namespace Plutonium
{
	/* Defines a basic directional light source. */
	struct DirectionalLight
		: Light
	{
	public:
		/* Initializes a new instance of a directional light source. */
		DirectionalLight(_In_ float yaw, _In_ float pitch, _In_ float roll, _In_ Color ambient, _In_ Color diffuse, _In_ Color specular)
			: Light(ambient, diffuse, specular)
		{
			SetDirection(yaw, pitch, roll);
		}

		DirectionalLight(_In_ const DirectionalLight &value) = delete;
		DirectionalLight(_In_ DirectionalLight &&value) = delete;

		_Check_return_ DirectionalLight& operator =(_In_ const DirectionalLight &other) = delete;
		_Check_return_ DirectionalLight& operator =(_In_ DirectionalLight &&other) = delete;

		/* Sets the direction of the light. */
		void SetDirection(_In_ float yaw, _In_ float pitch, _In_ float roll)
		{
			orien = Matrix::CreateRotation(yaw, pitch, roll);
		}

		/* Gets the orientation matrix associated with this light. */
		_Check_return_ const Matrix& GetOrientation(void) const
		{
			return orien;
		}

		/* Gets the direction of the light source. */
		_Check_return_ Vector3 GetDirection(void) const
		{
			return orien.GetRight();
		}
		/* Gets the up vector of the light source. */
		_Check_return_ Vector3 GetUp(void) const
		{
			return orien.GetUp();
		}

	private:
		Matrix orien;
	};
}