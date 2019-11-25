#pragma once
#include "UniformBlock.h"
#include "Content/PumLoader.h"
#include "Graphics/Vulkan/Shaders/Subpass.h"

namespace Pu
{
	/* Defines the specular glossiness material used by the model renderer. */
	class Material
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a PBR material from the specified descriptor pool for the specified subpass. */
		Material(DescriptorPool &pool);
		/* Initializes a new instance of a PBR material from the specified descriptor pool for the specified subpass with the specified parameters. */
		Material(DescriptorPool &pool, _In_ const PumMaterial &parameters);
		Material(_In_ const Material&) = delete;
		/* Move constructor. */
		Material(_In_ Material &&value);

		_Check_return_ Material& operator =(_In_ const Material&) = delete;
		/* Move assignment. */
		_Check_return_ Material& operator =(_In_ Material &&other);

		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ float specPower, _In_ Vector3 specular, _In_ Vector3 diffuse);
		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ float specPower, _In_ Color specular, _In_ Color diffuse);

		/* Sets all of the values for this material from a Plutonium model material. */
		inline void SetParameters(_In_ const PumMaterial &value)
		{
			SetParameters(value.Glossiness, value.SpecularPower, value.SpecularFactor, value.DiffuseFactor);
		}

		/* Sets the glossiness value for this material. */
		inline void SetGlossiness(_In_ float value)
		{
			roughness = 1.0f - value;
			IsDirty = true;
		}

		inline void SetSpecularPower(_In_ float value)
		{
			power = value;
			IsDirty = true;
		}

		/* Sets the specular factor for this material. */
		inline void SetSpecular(_In_ Vector3 value)
		{
			f0 = value;
			IsDirty = true;
		}

		/* Sets the specular factor for this material (alpha is ignored). */
		inline void SetSpecular(_In_ Color value)
		{
			SetSpecular(value.ToVector3());
		}

		/* Sets the diffuse factor for this material. */
		inline void SetDiffuse(_In_ Vector3 value)
		{
			diffuse = value;
			IsDirty = true;
		}

		/* Sets the diffuse factor for this material (alpha is ignored). */
		inline void SetDiffuse(_In_ Color value)
		{
			SetDiffuse(value.ToVector3());
		}

		/* Sets the diffuse texture for this material. */
		inline void SetDiffuse(_In_ const Texture &map)
		{
			Write(*diffuseMap, map);
		}

		/* Sets the specular glossiness texture for this material. */
		inline void SetSpecular(_In_ const Texture &map)
		{
			Write(*specularMap, map);
		}

		/* Sets the normal texture for this material. */
		inline void SetNormal(_In_ const Texture &map)
		{
			Write(*normalMap, map);
		}

		/* Gets the glossiness component of this material. */
		_Check_return_ inline float GetGlossiness(void) const
		{
			return 1.0f - roughness;
		}

		/* Gets the specular power component of this material. */
		_Check_return_ inline float GetSpecularPower(void) const
		{
			return power;
		}

		/* Gets the specular tint component of this material. */
		_Check_return_ inline Vector3 GetSpecularColor(void) const
		{
			return f0;
		}

	protected:
		/* Stages the buffer data for the uniform buffer. */
		virtual void Stage(_In_ byte *dest) override;

	private:
		Vector3 f0;
		Vector3 diffuse;
		float roughness, power;

		const Descriptor *diffuseMap, *specularMap, *normalMap;
	};
}