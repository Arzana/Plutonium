#pragma once
#include <map>
#include "Queue.h"
#include "Core/Collections/Vector.h"

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

		/* Gets the specific queue created with the logical device. */
		_Check_return_ inline Queue& GetQueue(_In_ uint32 familyIndex, _In_ uint32 queueIndex)
		{
			return queues.at(familyIndex).at(queueIndex);
		}

	private:
		friend class PhysicalDevice;

		InstanceHndl parent;
		DeviceHndl hndl;
		std::map<uint32, vector<Queue>> queues;

		PFN_vkDestroyDevice vkDestroyDevice;
		PFN_vkGetDeviceQueue vkGetDeviceQueue;		

		LogicalDevice(InstanceHndl parent, DeviceHndl hndl, uint32 queueCreateInfoCount, const DeviceQueueCreateInfo *queueCreateInfos);

		void LoadDeviceProcs(void);
		void Destory(void);
	};
}