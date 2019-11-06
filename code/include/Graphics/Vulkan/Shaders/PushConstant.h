#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Field.h"

namespace Pu
{
	class PhysicalDevice;

	/* Specifies information about a shaders push constant. */
	class PushConstant
		: public Field
	{
	public:
		/* Copy constructor. */
		PushConstant(_In_ const PushConstant&) = default;
		/* Move constructor. */
		PushConstant(_In_ PushConstant&&) = default;

		/* Copy assignment. */
		_Check_return_ PushConstant& operator =(_In_ const PushConstant&) = default;
		/* Move assignment. */
		_Check_return_ PushConstant& operator =(_In_ PushConstant&&) = default;

		/* Sets the offset of this push constant range in the push constant collection. */
		void SetOffset(_In_ size_t offset);

		/* Gets the size of this push constant. */
		_Check_return_ inline DeviceSize GetSize(void) const
		{
			return range.Size;
		}


	private:
		friend class Subpass;
		friend class Renderpass;

		PushConstantRange range;
		size_t maxRange;

		PushConstant(const FieldInfo &data);
		PushConstant(const PhysicalDevice &physicalDevice, const FieldInfo &data, ShaderStageFlag stage);
	};
}