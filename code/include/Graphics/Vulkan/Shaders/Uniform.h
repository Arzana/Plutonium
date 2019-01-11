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

		/* Gets the descriptor type currently assigned to the uniform. */
		_Check_return_ inline DescriptorType GetDescriptorType(void) const
		{
			return layoutBinding.DescriptorType;
		}

	private:
		friend class Renderpass;
		friend class GraphicsPipeline;
		friend class DescriptorSet;

		uint32 set;

		DescriptorSetLayoutBinding layoutBinding;

		Uniform(const FieldInfo &data, ShaderStageFlag stage);
	};
}