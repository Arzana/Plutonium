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
	layoutBinding.DescriptorCount = 1;

	if (GetInfo().Type.ComponentType == ComponentType::Image)
	{
		/* GLSL restrics us to only use combined descriptors. */
		layoutBinding.DescriptorType = DescriptorType::CombinedImageSampler;
	}
	else
	{
		/* Assume uniform buffer for other types as it's most used. */
		layoutBinding.DescriptorType = DescriptorType::UniformBuffer;

		/*
		https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
		The size of a value in a uniform buffer is determined as follows:
		Scalars (int, float, bool, etc)			4 bytes										Called N for further computations
		Vector									2 or 4 * N bytes							8 bytes for vec2 and 16 bytes for vec3 and vec4
		Arrays									Element Count * 16 bytes					Stored as multiple vec4's
		Matrices								Column Count * 16 bytes						Stored as array of column vec4's
		Struct									Sum(Components), Alligned to 16 bytes		Stored as the sum of it's components but padded to be 16 byte alligned. 
		*/

		if (GetInfo().ArrayElements == 1) size = GetInfo().Type.GetSize();
		else
		{
			size = max(sizeof(Vector4), GetInfo().Type.GetSize()) * GetInfo().ArrayElements;
		}
	}
}