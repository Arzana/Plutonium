#include <Graphics/Vulkan/Instance.h>
#include <Core/Diagnostics/Logging.h>
#include <Core/EnumUtils.h>

using namespace Pu;

int main(int, char**)
{
	VulkanInstance instance("VulkanTesting");

	constexpr float priorities[1] = { 1.0f };
	DeviceQueueCreateInfo queueInfo;
	DeviceCreateInfo deviceInfo;
	size_t selectedDevice = 0;

	const auto devices = instance.GetPhysicalDevices();
	for (decltype(auto) device : devices)
	{
		if (device.GetType() == PhysicalDeviceType::DiscreteGpu)
		{
			uint32 selectedFamily = 0;
			const auto queuefamilies = device.GetQueueFamilies();
			for (decltype(auto) family : queuefamilies)
			{
				if (_CrtEnumCheckFlag(family.Flags, QueueFlag::Graphics)) break;
				++selectedFamily;
			}

			if (selectedFamily >= queuefamilies.size()) Log::Fatal("Cannot create logical device without graphics queue!");

			queueInfo = DeviceQueueCreateInfo(selectedFamily, 1, priorities);
			deviceInfo = DeviceCreateInfo(1, &queueInfo);
			break;
		}
		
		++selectedDevice;
	}

	LogicalDevice device = devices.at(selectedDevice).CreateLogicalDevice(&deviceInfo);

	Log::PressAnyKeyToContinue();
	return 0;
}