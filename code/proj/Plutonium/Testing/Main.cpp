#include <Graphics/Platform/Windows/Win32Window.h>
#include <Core/Diagnostics/Logging.h>
#include <Core/EnumUtils.h>

using namespace Pu;

int main(int, char**)
{
	if (!VulkanInstance::AreExtensionsSupported({ u8"VK_KHR_surface", u8"VK_KHR_win32_surface" })) Log::Fatal("Platform doesn't support surface extension!");

	VulkanInstance instance("VulkanTesting", { u8"VK_KHR_surface", u8"VK_KHR_win32_surface" });
	NativeWindow *wnd = new Win32Window(instance, "TestGame", Vector2(600.0f));
	wnd->Show();

	constexpr float priorities[1] = { 1.0f };
	constexpr const char *deviceExtensions[1] = { u8"VK_KHR_swapchain" };
	DeviceQueueCreateInfo queueInfo;
	DeviceCreateInfo deviceInfo;
	size_t selectedDevice = 0;

	const auto devices = instance.GetPhysicalDevices();
	for (decltype(auto) device : devices)
	{
		if (device.GetType() == PhysicalDeviceType::DiscreteGpu && device.SupportsPlutonium())
		{
			uint32 selectedFamily = 0;
			const auto queuefamilies = device.GetQueueFamilies();
			for (decltype(auto) family : queuefamilies)
			{
				if (_CrtEnumCheckFlag(family.Flags, QueueFlag::Graphics) /*&&
					device.QueueFamilySupportsPresenting(selectedFamily, wnd->GetSurfaceH())*/)
				{
					break;
				}
				++selectedFamily;
			}

			if (selectedFamily >= queuefamilies.size()) Log::Fatal("Cannot create logical device without graphics queue!");

			queueInfo = DeviceQueueCreateInfo(selectedFamily, 1, priorities);
			deviceInfo = DeviceCreateInfo(1, &queueInfo);
			break;
		}
		
		++selectedDevice;
	}

	if (selectedDevice >= devices.size()) Log::Fatal("No usable GPU found!");
	deviceInfo.EnabledExtensionCount = 1;
	deviceInfo.EnabledExtensionNames = deviceExtensions;

	LogicalDevice device = devices.at(selectedDevice).CreateLogicalDevice(&deviceInfo);

	delete_s(wnd);
	Log::PressAnyKeyToContinue();
	return 0;
}