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
		Material(_In_ const Subpass &subpass, DescriptorPool &pool);
		Material(_In_ const Material&) = delete;
		/* Move constructor. */
		Material(_In_ Material &&value);

		_Check_return_ Material& operator =(_In_ const Material&) = delete;
		/* Move assignment. */
		_Check_return_ Material& operator =(_In_ Material &&other);

		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ Vector3 specular, _In_ Vector3 diffuse);
		/* Sets all of the values for this material. */
		void SetParameters(_In_ float glossiness, _In_ Color specular, _In_ Color diffuse);

		/* Sets all of the values for this material from a Plutonium model material. */
		inline void SetParameters(_In_ const PumMaterial &value)
		{
			SetParameters(value.Glossiness, value.SpecularFactor, value.DiffuseFactor);
		}

		/* Sets the glossiness value for this material. */
		inline void SetGlossiness(_In_ float value)
		{
			roughness = 1.0f - value;
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

	protected:
		/* Stages the buffer data for the uniform buffer. */
		virtual void Stage(_In_ byte *dest) override;

	private:
		Vector3 f0;
		Vector3 diffuse;
		float roughness;

		const Descriptor *diffuseMap;
	};
}