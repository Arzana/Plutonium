#include <Graphics/Platform/Windows/Win32Window.h>
#include <Graphics/Platform/GameWindow.h>
#include <Core/Diagnostics/Logging.h>
#include <Core/EnumUtils.h>

#include <Core/Threading/Tasks/Scheduler.h>
#include <Graphics/Vulkan/Renderpass.h>

using namespace Pu;

int main(int, char**)
{
	/* Create Vulkan instance. */
	VulkanInstance instance("VulkanTesting", { u8"VK_KHR_surface", u8"VK_KHR_win32_surface" });

	/* Create window. */
	NativeWindow *wnd = new Win32Window(instance, "TestGame", Vector2(600.0f));
	wnd->Show();

	constexpr float priorities[1] = { 1.0f };
	constexpr const char *deviceExtensions[1] = { u8"VK_KHR_swapchain" };
	size_t selectedDevice = 0;
	uint32 selectedFamily = 0;

	/* Get best physical device. */
	const auto devices = instance.GetPhysicalDevices();
	for (decltype(auto) device : devices)
	{
		if (device.GetType() == PhysicalDeviceType::DiscreteGpu && device.SupportsPlutonium())
		{
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
			break;
		}
		
		++selectedDevice;
	}

	DeviceQueueCreateInfo queueInfo(selectedFamily, 1, priorities);
	DeviceCreateInfo deviceInfo(1, &queueInfo);

	/* Ceate logical device. */
	if (selectedDevice >= devices.size()) Log::Fatal("No usable GPU found!");
	deviceInfo.EnabledExtensionCount = 1;
	deviceInfo.EnabledExtensionNames = deviceExtensions;
	LogicalDevice device = devices.at(selectedDevice).CreateLogicalDevice(&deviceInfo);

	TaskScheduler scheduler;
	Renderpass renderpass(device);
	Renderpass::LoadTask loader(renderpass, { "../assets/shaders/Triangle.vert", "../assets/shaders/Triangle.frag" });
	scheduler.Spawn(loader);

	while (!renderpass.IsLoaded()) {}

	/* Create game window. */
	GameWindow *gameWnd = new GameWindow(*wnd, device);
	while (wnd->testUpdate())
	{
		gameWnd->TestRun();
	}

	delete_s(gameWnd);
	delete_s(wnd);
	Log::PressAnyKeyToContinue();
	return 0;
}