#pragma once
#include "Graphics/Models/UniformBlock.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a directional light uniform block. */
	class DirectionalLight
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a directional light. */
		DirectionalLight(_In_ DescriptorPool &pool);
		DirectionalLight(_In_ const DirectionalLight&) = delete;
		/* Move constructor. */
		DirectionalLight(_In_ DirectionalLight &&value) = default;

		_Check_return_ DirectionalLight& operator =(_In_ const DirectionalLight&) = delete;
		/* Move assignment. */
		_Check_return_ DirectionalLight& operator =(_In_ DirectionalLight &&other) = default;

		/* Gets the direction of this light. */
		_Check_return_ inline Vector3 GetDirection(void) const
		{
			return orien.GetRight();
		}

		/* Gets the up vector of this light. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return orien.GetUp();
		}

		/* Gets the orientation of the light. */
		_Check_return_ inline const Quaternion& GetOrientation(void) const
		{
			return orien.GetOrientation();
		}

		/* Gets the view matrix from this lights point of view. */
		_Check_return_ inline const Matrix& GetView(void) const
		{
			return orien;
		}

		/* Gets the radiant color of the light. */
		_Check_return_ inline Vector3 GetRadiance(void) const
		{
			return radiance;
		}

		/* Gets the intensity of the light. */
		_Check_return_ inline float GetIntensity(void) const
		{
			return intensity;
		}

		/* Sets the direction of the light. */
		inline void SetDirection(_In_ float yaw, _In_ float pitch, _In_ float roll)
		{
			orien = Matrix::CreateRotation(yaw, pitch, roll);
			IsDirty = true;
		}

		/* Sets the direction of the light. */
		inline void SetDirection(_In_ Quaternion value)
		{
			orien = Matrix::CreateRotation(value);
		}

		/* Sets the color of the light. */
		inline void SetRadiance(_In_ Color value)
		{
			radiance = value.ToVector3();
			IsDirty = true;
		}

		/* Sets the intensity of the light. */
		inline void SetIntensity(_In_ float value)
		{
			intensity = value;
			IsDirty = true;
		}

	protected:
		/* Stages the lights properties to the buffer. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix orien;
		Vector3 radiance;
		float intensity;
	};
}