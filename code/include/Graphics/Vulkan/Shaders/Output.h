#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Field.h"
#include "OutputUsage.h"

namespace Pu
{
	/* Specifies information about a shader output field. */
	class Output
		: public Field
	{
	public:
		/* Sets the layout of the output to a specified value. */
		inline void SetLayout(_In_ ImageLayout layout)
		{
			reference.Layout = layout;
		}

		/* Sets how the output field should be handled, multisample can only be used if the usage is Color. */
		inline void SetUsage(_In_ OutputUsage usage, _In_opt_ bool multisample = false)
		{
			type = usage;
			if (type == OutputUsage::Color) resolve = multisample;
		}

		/* Sets the output to the specified swapchain format. */
		void SetDescription(_In_ const Swapchain &swapchain);

	private:
		friend class Renderpass;

		bool resolve;
		OutputUsage type;
		AttachmentReference reference;
		AttachmentDescription description;

		Output(const FieldInfo &data, uint32 attachment);
	};
}