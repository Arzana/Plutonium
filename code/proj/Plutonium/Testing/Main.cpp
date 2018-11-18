#include <Graphics/Platform/Windows/Win32Window.h>
#include <Graphics/Platform/GameWindow.h>
#include <Core/Diagnostics/Logging.h>
#include <Core/EnumUtils.h>
#include <Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h>
#include <Core/Threading/ThreadUtils.h>

using namespace Pu;

int main(int, char**)
{
	string dst = SPIRV::FromGLSLPath("../assets/shaders/Triangle.vert");


	/* Create Vulkan instance. */
	VulkanInstance instance("VulkanTesting", { u8"VK_KHR_surface", u8"VK_KHR_win32_surface" });

	/* Create window. */
	NativeWindow *wnd = new Win32Window(instance, "TestGame", Vector2(600.0f));
	wnd->Show();

	constexpr float priorities[1] = { 1.0f };
	constexpr const char *deviceExtensions[1] = { u8"VK_KHR_swapchain" };
	size_t selectedDevice = 0, selectedFamily = 0;

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