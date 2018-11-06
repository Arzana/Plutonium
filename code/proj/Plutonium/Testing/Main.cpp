#include <Graphics/Vulkan/Instance.h>
#include <Core/Diagnostics/Logging.h>

using namespace Pu;

int main(int, char**)
{
	VulkanInstance instance("VulkanTesting");

	const auto devices = instance.GetPhysicalDevices();
	for (decltype(auto) device : devices)
	{
		const auto queueFamilies = device.GetQueueFamilies();
		auto[major, minor, patch] = device.GetDriverVersion();

		for (const auto family : queueFamilies)
		{
			Log::Verbose("%s (v%u.%u.%u) queue family has %u queue(s).", device.GetName(), major, minor, patch, family.QueueCount);
		}
	}

	Log::PressAnyKeyToContinue();
	return 0;
}