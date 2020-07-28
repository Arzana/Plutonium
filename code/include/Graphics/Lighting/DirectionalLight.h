#pragma once
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Core/Math/Matrix.h"
#include "Graphics/Textures/TextureCube.h"

namespace Pu
{
	/* Defines a directional light uniform block. */
	class DirectionalLight
		: public DescriptorSet
	{
	public:
		/* Initializes a new instance of a directional light. */
		DirectionalLight(_In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout);
		DirectionalLight(_In_ const DirectionalLight&) = delete;
		/* Move constructor. */
		DirectionalLight(_In_ DirectionalLight &&value) = default;

		_Check_return_ DirectionalLight& operator =(_In_ const DirectionalLight&) = delete;
		/* Move assignment. */
		_Check_return_ DirectionalLight& operator =(_In_ DirectionalLight &&other) = default;

		/* Attempts to render a ImGUI window for this directional light. */
		void RenderGUI(void);

		/* Gets the direction of this light. */
		_Check_return_ inline Vector3 GetDirection(void) const
		{
			return orien.GetForward();
		}

		/* Gets the up vector of this light. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return orien.GetUp();
		}

		/* Gets the orientation of the light. */
		_Check_return_ inline Quaternion GetOrientation(void) const
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
			return radiance.XYZ;
		}

		/* Gets the intensity of the light. */
		_Check_return_ inline float GetIntensity(void) const
		{
			return radiance.W;
		}

		/* Sets the direction of the light from euler angles. */
		inline void SetDirection(_In_ Vector3 euler)
		{
			SetDirection(euler.X, euler.Y, euler.Z);
		}

		/* Sets the direction of the light. */
		inline void SetDirection(_In_ float yaw, _In_ float pitch, _In_ float roll)
		{
			orien = Matrix::CreateRotation(pitch, yaw, roll);
		}

		/* Sets the direction of the light. */
		inline void SetDirection(_In_ Quaternion value)
		{
			orien = Matrix::CreateRotation(value);
		}

		/* Sets the color of the light. */
		inline void SetRadiance(_In_ Color value)
		{
			radiance.XYZ = value.ToVector3();
		}

		/* Sets the intensity of the light. */
		inline void SetIntensity(_In_ float value)
		{
			radiance.W = value;
		}

		/* Sets the environment map used by this light source. */
		inline void SetEnvironment(_In_ const TextureCube &probe)
		{
			Write(*envi, probe);
		}

	protected:
		/* Stages the lights properties to the buffer. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix orien;
		Vector4 radiance;
		const Descriptor *envi;
	};
}