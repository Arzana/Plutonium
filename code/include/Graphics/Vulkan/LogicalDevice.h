#pragma once
#include "VulkanProcedres.h"

namespace Pu
{
	/* Defines a Vulkan logical device. */
	class LogicalDevice
	{
	public:
		LogicalDevice(_In_ const LogicalDevice&) = delete;
		/* Move constructor. */
		LogicalDevice(_In_ LogicalDevice &&value);
		/* Releases the logical device. */
		~LogicalDevice(void)
		{
			Destory();
		}

		_Check_return_ LogicalDevice& operator =(_In_ const LogicalDevice&) = delete;
		/* Move assignment. */
		_Check_return_ LogicalDevice& operator =(_In_ LogicalDevice &&other);

	private:
		friend class PhysicalDevice;

		InstanceHndl parent;
		DeviceHndl hndl;
		PFN_vkDestroyDevice vkDestroyDevice;

		LogicalDevice(InstanceHndl parent, DeviceHndl hndl);

		void Destory(void);
	};
}