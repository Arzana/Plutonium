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
	: Field(data), physicalDevice(&physicalDevice), set(data.Decorations.Numbers.at(spv::Decoration::DescriptorSet))
{
	layoutBinding.Binding = GetInfo().Decorations.Numbers.at(spv::Decoration::Binding);
	layoutBinding.StageFlags = stage;
	layoutBinding.DescriptorCount = max(1u, GetInfo().ArrayElements);

	if (GetInfo().Type.ComponentType == ComponentType::Image)
	{
		/* GLSL restrics us to only use combined descriptors. */
		layoutBinding.DescriptorType = DescriptorType::CombinedImageSampler;
	}
	else
	{
		/* Assume uniform buffer for other types as it's most used. */
		layoutBinding.DescriptorType = DescriptorType::UniformBuffer;
		size = static_cast<DeviceSize>(GetInfo().Type.GetSize());
	}
}