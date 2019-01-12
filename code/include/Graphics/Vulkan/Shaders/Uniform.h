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

		/* Gets the offset specified for the uniform buffer member (if not in uniform buffer; 0). */
		_Check_return_ inline DeviceSize GetOffset(void) const 
		{
			return static_cast<DeviceSize>(Info.Decorations.MemberOffset);
		}

		/* Gets the size (in bytes) of the uniform (if not aplicable; 0). */
		_Check_return_ inline DeviceSize GetSize(void) const 
		{
			return static_cast<DeviceSize>(sizeof_fieldType(Info.Type));
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