#include "Application.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/Profiler.h"
#include "Graphics/Vulkan/Instance.h"
#include "Core/Diagnostics/Memory.h"
#include "Core/Threading/PuThread.h"
#include "Streams/RuntimeConfig.h"
#include "Core/EnumUtils.h"
#include <imgui/include/imgui.h>

#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
#endif

Pu::Application::Application(const wstring & name)
	: IsFixedTimeStep(true), suppressUpdate(false), name(name),
	targetElapTimeFocused(ApplicationFocusedTargetTime), targetElapTimeBackground(ApplicationNoFocusTargetTime),
	maxElapTime(ApplicationMaxLagCompensation), accumElapTime(0.0f), gameTime(), device(nullptr), initialized(false)
{
	InitializePlutonium();
	TaskScheduler::Start();
	input = new InputDeviceHandler();

	/* Initialize ImGui if needed. */
	if constexpr (ImGuiAvailable)
	{
#ifdef _DEBUG
		IMGUI_CHECKVERSION();
#endif
		ImGui::CreateContext();
		Profiler::SetTargetFrameTime(ApplicationFocusedTargetTime);
	}
}

Pu::Application::~Application(void)
{
	TaskScheduler::StopWait();

	delete input;
	delete device;
	delete instance;

	/* Finalize ImGui if needed. */
	if constexpr (ImGuiAvailable) ImGui::DestroyContext();

	/* Make sure the debugging symbols are freed. */
#ifdef _WIN32
	_CrtFinalizeWinProcess();
#endif
}

void Pu::Application::Run(void)
{
	/* Initialize application. */
	gameTime.Start();
	DoInitialize();

	/* Load content. */
	prevTime = gameTime.SecondsAccurate();
	LoadContent(*content);

	/* Run application loop. */
	while (wnd->Update())
	{
		while (!Tick());
	}

	/* Make sure to finalize the game window before allowing the user to release their resources. */
	gameWnd->Finalize();

	/* Finalize application. */
	UnLoadContent(*content);
	DoFinalize();
}

void Pu::Application::AddSystem(System * component)
{
	/* Add the component to the list and initialize it if needed. */
	systems.emplace_back(component);
	
	/* Initializes the component and sort the list again if we've already initialized the application. */
	if (initialized)
	{
		component->DoInitialize();
		systems.sort(System::SortPredicate);
	}
}

void Pu::Application::RemoveSystem(System & system)
{
	for (size_t i = 0; i < systems.size(); i++)
	{
		System *cur = systems[i];
		if (cur == &system)
		{
			delete cur;
			systems.removeAt(i);
			return;
		}
	}
}

void Pu::Application::SetTargetTimeStep(int32 hertz)
{
	targetElapTimeFocused = 1.0f / hertz;
	Profiler::SetTargetFrameTime(targetElapTimeFocused);
	targetElapTimeBackground = min(targetElapTimeFocused, targetElapTimeBackground);
}

void Pu::Application::InitializePlutonium(void)
{
	/* Make sure this is only called once. */
	static bool called = false;
	if (called) return;
	called = true;

	/* Seed random. */
	srand(static_cast<uint32>(time(nullptr)));

	/* 
	Set the current threads name to the main thread. 
	Also lock the main thread to the guaranteed core.
	*/
	PuThread::SetName(L"PuMain");
	PuThread::Lock(0);
}

void Pu::Application::InitializeVulkan(void)
{
	constexpr float PRIORITIES[2] = { 1.0f, 1.0f };

	/* 
	Create the Vulkan instance, we need the surface extensions for the native window. 
	Additional color spaces are nice to support but we can do with just sRGB.
	Additional information about physical devices can also come in handy, but it's not needed.
	*/
	instance = new VulkanInstance(name.toUTF8().c_str(), LogAvailableVulkanExtensionsAndLayers,
		{
			u8"VK_KHR_surface",
#if defined(_DEBUG) || defined(VULKAN_FORCE_VALIDATION)
			u8"VK_EXT_debug_utils",
#endif
#ifdef _WIN32
			u8"VK_KHR_win32_surface",
#endif
		},
		{
			u8"VK_EXT_swapchain_colorspace",
			u8"VK_KHR_get_physical_device_properties2",
			u8"VK_KHR_get_surface_capabilities2"
		});

	Profiler::GetInstance().vkInstance = instance;

	/* Create the native window. */
#ifdef _WIN32
	wnd = new Win32Window(*instance, name);
#else
	Log::Fatal("Unable to create window on this platform!");
#endif

	/* Choose a physical device and get graphics and transfer queue families. */
	const PhysicalDevice &physicalDevice = ChoosePhysicalDevice();
	const uint32 graphicsQueueFamily = physicalDevice.GetBestGraphicsQueueFamily(wnd->GetSurface());
	const uint32 transferQueueFamily = physicalDevice.GetBestTransferQueueFamily();
	const uint32 computeQueueFamily = physicalDevice.GetBestComputeQueuFamily();

	/*
	Specify the amount of queues (adding one if they are the same family):
	- At least 2 graphics queues, one used for rendering (1) and one for asset loading (0).
	- At least 1 compute queue.
	- 1 transfer queue, only used for asset streaming.
	*/
	const DeviceQueueCreateInfo queueCreateInfos[] =
	{
		DeviceQueueCreateInfo(graphicsQueueFamily, 2, PRIORITIES),
		DeviceQueueCreateInfo(computeQueueFamily, 1, PRIORITIES),
		DeviceQueueCreateInfo(transferQueueFamily, 1, PRIORITIES)
	};

	/* 
	Allow the user to enable specific physical device features or extensions.
	The Swapchain extension is enabled by default as the GameWindow requires it.
	All the extensions enabled are later tested to see if the implementation actually supports them.
	*/
	vector<const char*> deviceExtensions = { u8"VK_KHR_swapchain" };
	EnableFeatures(physicalDevice.GetSupportedFeatures(), const_cast<PhysicalDeviceFeatures&>(physicalDevice.enabledFeatures.Features), deviceExtensions);
	for (size_t i = 0; i < deviceExtensions.size();)
	{
		if (physicalDevice.IsExtensionSupported(deviceExtensions[i])) ++i;
		else deviceExtensions.removeAt(i);
	}

	/* Create logical device. */
	DeviceCreateInfo deviceCreateInfo{ 3, queueCreateInfos, deviceExtensions, physicalDevice.enabledFeatures };
	device = physicalDevice.CreateLogicalDevice(deviceCreateInfo);
	device->SetQueues(graphicsQueueFamily, computeQueueFamily, transferQueueFamily);

	/* Log the available memory, useful for system debugging. */
	const uint64 dram = MemoryFrame::GetCPUMemStats().TotalRam;
	const uint64 vram = physicalDevice.GetDeviceLocalBytes();
	Log::Message("Available memory: DRAM: %u GB, VRAM: %u GB", b2gb(dram), b2gb(vram));
}

const Pu::PhysicalDevice & Pu::Application::ChoosePhysicalDevice(void)
{
	/* Get window surface to check for presenting. */
	const Surface &surface = wnd->GetSurface();
	const PhysicalDevice *choosen = nullptr;
	uint32 highscore = 0;

	/* Loop through all physical devices to find the best one. */
	for(const PhysicalDevice &physicalDevice : instance->GetPhysicalDevices())
	{
		/* Check if the physical device supports our framework. */
		if (physicalDevice.SupportsPlutonium(surface))
		{
			/* Ask user if the current physical device is usable. */
			if (GpuPredicate(physicalDevice))
			{
				uint32 score = 1;

				/* For more information about scoring see document 'Scoring'. */
				if (physicalDevice.GetType() == PhysicalDeviceType::DiscreteGpu) score += 6;
				else if (physicalDevice.GetType() == PhysicalDeviceType::IntegratedGpu) score += 3;

				for (const QueueFamilyProperties &family : physicalDevice.GetQueueFamilies())
				{
					if ((family.Flags & QueueFlags::TypeMask) == QueueFlags::Transfer) ++score;
					else if ((family.Flags & QueueFlags::TypeMask) == QueueFlags::Compute) ++score;
				}

				/* Set new highscore and update choosen physical device if needed. */
				if (score > highscore)
				{
					highscore = score;
					choosen = &physicalDevice;
				}
			}
		}
	}

	/* We cannot return a random physical device as that would not satisfy the conditions. */
	if (!choosen) Log::Fatal("No Vulkan/Plutonium compatible physical device could be found!");
	return *choosen;
}

bool Pu::Application::Tick(void)
{
	/* Update timers. */
	const float curTime = gameTime.SecondsAccurate();
	accumElapTime += curTime - prevTime;
	prevTime = curTime;

	/* If the update rate if fixed and we're bolow the threshold; halt the thread's execution. */
	const float targetElapTime = wnd->HasFocus() ? targetElapTimeFocused : targetElapTimeBackground;
	if (IsFixedTimeStep && accumElapTime < targetElapTime)
	{
		const uint64 sleepTime = static_cast<uint64>((targetElapTime - accumElapTime) * 1000.0f);
		PuThread::Sleep(sleepTime);
		return false;
	}

	/* Make sure we don't update too much. */
	if (accumElapTime > maxElapTime) accumElapTime = maxElapTime;
	float dt = 0.0f;

	/* Catch up on game updates. */
	if (suppressUpdate) suppressUpdate = false;
	else
	{
		if (IsFixedTimeStep)
		{
			/* Do updates. */
			for (; accumElapTime >= targetElapTime; accumElapTime -= targetElapTime)
			{
				dt += targetElapTime;
				DoUpdate(targetElapTime);
			}
		}
		else
		{
			/* Do updates. */
			dt = accumElapTime;
			accumElapTime = 0.0f;
			DoUpdate(dt);
		}
	}

	/* Do frame render. */
	if (!wnd->shouldSuppressRender) DoRender(dt);

	return true;
}

void Pu::Application::DoInitialize(void)
{
	/* Create the Vulkan instance and logical device and register the raw input handles in the window. */
	InitializeVulkan();
#ifdef _WIN32
	input->RegisterInputDevicesWin32(*dynamic_cast<Win32Window*>(wnd));
#endif

	/* The fetcher needs to be created here as it needs the logical device. */
	content = new AssetFetcher(*device);
	saver = new AssetSaver(*device);

	/* Set the default mode for the Window before the swapchain is created, this allows for seamless window transitions. */
	wnd->SetMode(RuntimeConfig::QueryEnum(L"WindowMode", WindowMode::Borderless));
	gameWnd = new GameWindow(*wnd, *device);

	/* Move the log window out of the way on debug mode. */
#ifdef _DEBUG
	_CrtMoveDebugTerminal(*wnd);
#endif

	/* Window is shown a second time to give it focus. */
	Initialize();
	wnd->Show();
	
	/* Initialize and sort the systems. */
	for (System *cur : systems) cur->DoInitialize();
	systems.sort(System::SortPredicate);
	initialized = true;
}

void Pu::Application::DoFinalize(void)
{
	/* Finalize all components that are set to update before the application update. */
	size_t i = 0;
	for (; i < systems.size(); i++)
	{
		System *cur = systems[i];
		if (cur->place > 0) break;
		cur->Finalize();
		delete cur;
	}

	Finalize();

	/* Finalize all components that are set to update after the application update. */
	for (; i < systems.size(); i++)
	{
		systems[i]->Finalize();
		delete systems[i];
	}

	delete content;
	delete saver;
	delete gameWnd;
	delete wnd;
}

void Pu::Application::DoUpdate(float dt)
{
	/* Update all components that are set to update before the application update. */
	size_t i = 0;
	for (; i < systems.size(); i++)
	{
		System *cur = systems[i];
		if (cur->place > 0) break;
		cur->Update(dt);
	}

	Update(dt);

	/* Update all components that are set to update after the application update. */
	for (; i < systems.size(); i++) systems[i]->Update(dt);
}

void Pu::Application::BeginRender(void)
{
	/* Start the current command buffer and call pre-render. */
	gameWnd->BeginRender();
	PreRender();
}

void Pu::Application::DoRender(float dt)
{
	lastDt = dt;

	BeginRender();
	Render(dt, gameWnd->GetCommandBuffer());
	EndRender();
}

void Pu::Application::EndRender(void)
{
	/* End the current command buffer and call post-render. */
	PostRender();
	gameWnd->EndRender();
}