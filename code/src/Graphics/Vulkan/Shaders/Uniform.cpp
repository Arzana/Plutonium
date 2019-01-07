#include "Graphics/Vulkan/Shaders/Uniform.h"

Pu::Uniform::Uniform(const FieldInfo & data, ShaderStageFlag stage)
	: Field(data), hndl(nullptr), set(data.Decorations.Numbers.at(spv::Decoration::DescriptorSet))
{
	layoutBinding.Binding = Info.Decorations.Numbers.at(spv::Decoration::Binding);
	layoutBinding.StageFlags = stage;
	layoutBinding.DescriptorCount = max(1u, Info.ArrayElements);

	switch (Info.Type)
	{
	case (FieldTypes::Image1D):
	case (FieldTypes::Image2D):
	case (FieldTypes::Image3D):
	case (FieldTypes::ImageCube):
		isSampler = true;	// GLSL restrics us to only use combined descriptors.
		layoutBinding.DescriptorType = DescriptorType::CombinedImageSampler;
		break;
	default: 
		isSampler = false;	// Assume uniform buffer for other types as it's most used.
		layoutBinding.DescriptorType = DescriptorType::UniformBuffer;
		break;
	}
}

void Pu::Uniform::CheckIfSampler(void) const
{
	if (!isSampler) Log::Warning("Attempting to set value for non-sampler that is only used with samplers!");
}