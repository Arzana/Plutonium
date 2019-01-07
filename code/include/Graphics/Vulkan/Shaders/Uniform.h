#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Field.h"

namespace Pu
{
	/* Specifies information about a shaders uniform constants. */
	class Uniform
		: public Field
	{
	public:
		/* Overrides the default descriptor type for this uniform. */
		inline void SetDescriptor(_In_ DescriptorType type)
		{
			layoutBinding.DescriptorType = type;
		}

		/* Sets the filter used for minification lookups. */
		inline void SetMinification(_In_ Filter filter)
		{
			CheckIfSampler();
			createInfo.MinFilter = filter;
		}

		/* Sets the filter used for magnification lookups. */
		inline void SetMagnification(_In_ Filter filter)
		{
			CheckIfSampler();
			createInfo.MagFilter = filter;
		}

		/* Sets the filter used for mipmap lookups.  */
		inline void SetMipmap(_In_ SamplerMipmapMode mode)
		{
			CheckIfSampler();
			createInfo.MipmapMode = mode;
		}

		/* Sets the addressing mode for coordinates outside of the 0 to 1 range. */
		inline void SetWrapping(_In_ SamplerAddressMode mode)
		{
			CheckIfSampler();
			createInfo.AddressModeU = mode;
			createInfo.AddressModeV = mode;
			createInfo.AddressModeW = mode;
		}

		/* Sets whether anisotropic filtering is enabled and what the maximum value is. */
		inline void SetAnisotropy(_In_ bool enabled, _In_ float max = 0.0f)
		{
			CheckIfSampler();
			createInfo.AnisotropyEnable = enabled;
			createInfo.MaxAnisotropy = max;
		}

		/* Gets the descriptor type currently assigned to the uniform. */
		_Check_return_ inline DescriptorType GetDescriptorType(void) const
		{
			return layoutBinding.DescriptorType;
		}

	private:
		friend class Renderpass;
		friend class GraphicsPipeline;
		friend class DescriptorSet;

		bool isSampler;
		SamplerHndl hndl;
		uint32 set;

		SamplerCreateInfo createInfo;
		DescriptorSetLayoutBinding layoutBinding;

		Uniform(const FieldInfo &data, ShaderStageFlag stage);
		void CheckIfSampler(void) const;
	};
}