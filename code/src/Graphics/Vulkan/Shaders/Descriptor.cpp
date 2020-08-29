#include "Graphics/Vulkan/Shaders/Descriptor.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::DeviceSize Pu::Descriptor::GetAllignedOffset(DeviceSize offset) const
{
	return physicalDevice ? physicalDevice->GetUniformBufferOffsetAllignment(offset) : 0;
}

Pu::Descriptor::Descriptor(const FieldInfo & data)
	: Field(data), physicalDevice(nullptr), set(0)
{}

Pu::Descriptor::Descriptor(const PhysicalDevice &physicalDevice, const FieldInfo & data, ShaderStageFlag stage)
	: Field(data), physicalDevice(&physicalDevice), size(0),
	set(data.Decorations.Numbers.at(spv::Decoration::DescriptorSet))
{
	layoutBinding.Binding = data.Decorations.Numbers.at(spv::Decoration::Binding);
	layoutBinding.StageFlags = stage;
	layoutBinding.DescriptorCount = 1;

	if (data.Decorations.Contains(spv::Decoration::InputAttachmentIndex))
	{
		/* The descriptor is an input attachment. */
		layoutBinding.DescriptorType = DescriptorType::InputAttachment;
	}
	else if (data.Type.ComponentType == ComponentType::SampledImage)
	{
		/* GLSL restrics us to only use combined descriptors. */
		layoutBinding.DescriptorType = DescriptorType::CombinedImageSampler;
	}
	else if (data.Type.ComponentType == ComponentType::StoreImage)
	{
		layoutBinding.DescriptorType = DescriptorType::StorageImage;
	}
	else
	{
		/* Assume uniform buffer for other types as it's most used. */
		layoutBinding.DescriptorType = DescriptorType::UniformBuffer;
		size = Field::GetSize();
	}
}