#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Field.h"

namespace Pu
{
	class PhysicalDevice;

	/* Specifies information about a shaders uniform constants. */
	class Descriptor
		: public Field
	{
	public:
		/* Copy constructor. */
		Descriptor(_In_ const Descriptor&) = default;
		/* Move constructor. */
		Descriptor(_In_ Descriptor&&) = default;

		/* Copy assignment. */
		_Check_return_ Descriptor& operator =(_In_ const Descriptor&) = default;
		/* Move assignment. */
		_Check_return_ Descriptor& operator =(_In_ Descriptor&&) = default;

		/* Overrides the default descriptor type for this uniform. */
		inline void SetType(_In_ DescriptorType type)
		{
			layoutBinding.DescriptorType = type;
		}

		/* Gets the descriptor type currently assigned to the uniform. */
		_Check_return_ inline DescriptorType GetType(void) const
		{
			return layoutBinding.DescriptorType;
		}

		/* Gets the offset specified for the uniform buffer member (if not in uniform buffer; 0). */
		_Check_return_ inline DeviceSize GetOffset(void) const 
		{
			return static_cast<DeviceSize>(GetInfo().Decorations.MemberOffset);
		}

		/* Gets the size (in bytes) of the uniform with allignment (if not aplicable; 0). */
		_Check_return_ inline DeviceSize GetSize(void) const 
		{
			return size;
		}

		/* Gets the parent set for this descriptor. */
		_Check_return_ inline uint32 GetSet(void) const
		{
			return set;
		}

		/* Gets the binding of the descriptor. */
		_Check_return_ inline uint32 GetBinding(void) const
		{
			return layoutBinding.Binding;
		}

		/* Gets the offset (in bytes) required after this descriptor. */
		_Check_return_ DeviceSize GetAllignedOffset(_In_ DeviceSize offset) const;

	private:
		friend class Subpass;
		friend class Pipeline;
		friend class DescriptorSet;

		const PhysicalDevice *physicalDevice;

		uint32 set;
		DeviceSize size;

		DescriptorSetLayoutBinding layoutBinding;

		Descriptor(const FieldInfo &data);
		Descriptor(const PhysicalDevice &physicalDevice, const FieldInfo &data, ShaderStageFlag stage);
	};
}