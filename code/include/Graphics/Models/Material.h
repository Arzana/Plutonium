#pragma once
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Textures/Texture2D.h"
#include "Content/PumLoader.h"

namespace Pu
{
	/* Defines the specular glossiness material used by the model renderer. */
	class Material
		: public DescriptorSet
	{
	public:
		/* Initializes a new instance of a PBR material from the specified descriptor pool for the specified subpass. */
		Material(_In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout);
		/* Initializes a new instance of a PBR material from the specified descriptor pool for the specified subpass with the specified parameters. */
		Material(_In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout, _In_ const PumMaterial &parameters);
		Material(_In_ const Material&) = delete;
		/* Move constructor. */
		Material(_In_ Material &&value) = default;

		_Check_return_ Material& operator =(_In_ const Material&) = delete;
		/* Move assignment. */
		_Check_return_ Material& operator =(_In_ Material &&other) = default;

		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ float specPower, _In_ Vector3 specular, _In_ Vector3 diffuse, _In_ float threshold);
		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ float specPower, _In_ Color specular, _In_ Color diffuse, _In_ float theshold);

		/* Sets all of the values for this material from a Plutonium model material. */
		inline void SetParameters(_In_ const PumMaterial &value)
		{
			SetParameters(value.Glossiness, value.SpecularPower, value.SpecularFactor, value.DiffuseFactor, value.AlphaTheshold);
		}

		/* Sets the glossiness value for this material. */
		inline void SetGlossiness(_In_ float value)
		{
			diffuse.W = 1.0f - value;
		}

		/* Sets the specular power of this material. */
		inline void SetSpecularPower(_In_ float value)
		{
			specular.W = value;
		}

		/* Sets the specular factor for this material. */
		inline void SetSpecular(_In_ Vector3 value)
		{
			specular.XYZ = value;
		}

		/* Sets the specular factor for this material (alpha is ignored). */
		inline void SetSpecular(_In_ Color value)
		{
			SetSpecular(value.ToVector3());
		}

		/* Sets the diffuse factor for this material. */
		inline void SetDiffuse(_In_ Vector3 value)
		{
			diffuse.XYZ = value;
		}

		/* Sets the diffuse factor for this material (alpha is ignored). */
		inline void SetDiffuse(_In_ Color value)
		{
			SetDiffuse(value.ToVector3());
		}

		/* Sets the alpha threshold for this material. */
		inline void SetAlphaThreshold(_In_ float value)
		{
			threshold = value;
		}

		/* Sets the diffuse texture for this material. */
		inline void SetDiffuse(_In_ const Texture2D &map)
		{
			Write(*diffuseMap, map);
		}

		/* Sets the specular glossiness texture for this material. */
		inline void SetSpecular(_In_ const Texture2D &map)
		{
			Write(*specularMap, map);
		}

		/* Sets the normal texture for this material. */
		inline void SetNormal(_In_ const Texture2D &map)
		{
			Write(*normalMap, map);
		}

		/* Gets the glossiness component of this material. */
		_Check_return_ inline float GetGlossiness(void) const
		{
			return 1.0f - diffuse.W;
		}

		/* Gets the specular power component of this material. */
		_Check_return_ inline float GetSpecularPower(void) const
		{
			return specular.W;
		}

		/* Gets the specular tint component of this material. */
		_Check_return_ inline Vector3 GetSpecularColor(void) const
		{
			return specular.XYZ;
		}

		/* Gets the alpha threshold of this material.s */
		_Check_return_ inline float GetAlphaThreshold(void) const
		{
			return threshold;
		}

	protected:
		/* Stages the buffer data for the uniform buffer. */
		void Stage(_In_ byte *dest) final;

	private:
		Vector4 specular;
		Vector4 diffuse;
		float threshold;

		const Descriptor *diffuseMap, *specularMap, *normalMap;
	};
}