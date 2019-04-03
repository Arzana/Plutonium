#include "Graphics/Vulkan/Shaders/Uniform.h"

Pu::Uniform::Uniform(const FieldInfo & data, ShaderStageFlag stage)
	: Field(data), set(data.Decorations.Numbers.at(spv::Decoration::DescriptorSet))
{
	layoutBinding.Binding = Info.Decorations.Numbers.at(spv::Decoration::Binding);
	layoutBinding.StageFlags = stage;
	layoutBinding.DescriptorCount = max(1u, Info.ArrayElements);

	if (Info.Type.ComponentType == ComponentType::Image)
	{
		/* GLSL restrics us to only use combined descriptors. */
		layoutBinding.DescriptorType = DescriptorType::CombinedImageSampler;
	}
	else
	{
		/* Assume uniform buffer for other types as it's most used. */
		layoutBinding.DescriptorType = DescriptorType::UniformBuffer;
	}
}