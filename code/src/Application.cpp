#include "Application.h"
#include "Core/Threading/ThreadUtils.h"
#include "Graphics/Vulkan/Instance.h"
#include "Core/EnumUtils.h"
#include "Core/Diagnostics/DbgUtils.h"

#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
#endif

Pu::Application::Application(const wstring & name, float width, float height)
	: IsFixedTimeStep(true), suppressUpdate(false), name(name), initialWndSize(width, height),
	targetElapTimeFocused(ApplicationFocusedTargetTime), targetElapTimeBackground(ApplicationNoFocusTargetTime),
	maxElapTime(ApplicationMaxLagCompensation), accumElapTime(0.0f), gameTime(), device(nullptr), initialized(false)
{
	InitializePlutonium();
	scheduler = new TaskScheduler();
	input = new InputDeviceHandler();
}

Pu::Application::~Application(void)
{
	delete input;
	delete scheduler;

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
	LoadContent();
	Log::Verbose("Finished initializing and loading content for '%ls', took %f seconds.", wnd->GetTitle().c_str(), gameTime.SecondsAccurate());

	/* Run application loop. */
	while (wnd->Update())
	{
		while (!Tick());
	}

	/* Make sure to finalize the game window before allowing the user to release their resources. */
	gameWnd->Finalize();

	/* Finalize application. */
	UnLoadContent();
	DoFinalize();
}

void Pu::Application::AddComponent(Component * component)
{
	/* Add the component to the list and initialize it if needed. */
	components.emplace_back(component);
	
	/* Initializes the component and sort the list again if we've already initialized the application. */
	if (initialized)
	{
		component->DoInitialize();
		std::sort(components.begin(), components.end(), Component::SortPredicate);
	}
}

void Pu::Application::RemoveComponent(Component & component)
{
	for (size_t i = 0; i < components.size(); i++)
	{
		Component *cur = components[i];
		if (cur == &component)
		{
			delete cur;
			components.removeAt(i);
			return;
		}
	}
}

void Pu::Application::InitializePlutonium(void)
{
	/* Make sure this is only called once. */
	static bool called = false;
	if (called) return;
	called = true;

	/* Seed random. */
	srand(static_cast<uint32>(time(nullptr)));

	/* Set the current threads name to the main thread. */
	_CrtSetCurrentThreadName(L"PuMain");
}

void Pu::Application::InitializeVulkan(void)
{
	constexpr const char *DEVICE_EXTENSIONS[1] = { u8"VK_KHR_swapchain" };
	constexpr float PRIORITIES[1] = { 1.0f };

	/* Create the Vulkan instance, ew need the surface extensions for the native window. */
	instance = new VulkanInstance(name.toUTF8().c_str(),
		{
			u8"VK_KHR_surface",
#ifdef _DEBUG
			u8"VK_EXT_debug_utils",
#endif
#ifdef _WIN32
			u8"VK_KHR_win32_surface",
#endif
		});

	/* Create the native window. */
#ifdef _WIN32
	wnd = new Win32Window(*instance, name, initialWndSize);
#else
	Log::Fatal("Unable to create window on this platform!");
#endif

	/* Choose a physical device and get graphics and transfer queue families. */
	const PhysicalDevice &physicalDevice = ChoosePhysicalDevice();
	const uint32 graphicsQueueFamily = physicalDevice.GetBestGraphicsQueueFamily(wnd->GetSurface());
	const uint32 transferQueueFamily = physicalDevice.GetBestTransferQueueFamily();

	const DeviceQueueCreateInfo queueCreateInfos[] =
	{
		DeviceQueueCreateInfo(graphicsQueueFamily, 1, PRIORITIES),
		DeviceQueueCreateInfo(transferQueueFamily, 1, PRIORITIES)
	};

	/* Create logical device. */
	DeviceCreateInfo deviceCreateInfo(2, queueCreateInfos, 1, DEVICE_EXTENSIONS);
	device = physicalDevice.CreateLogicalDevice(&deviceCreateInfo);
	device->SetQueues(graphicsQueueFamily, transferQueueFamily);
}

const Pu::PhysicalDevice & Pu::Application::ChoosePhysicalDevice(void)
{
	/* Get window surface to check for presenting. */
	const Surface &surface = wnd->GetSurface();

	/* Loop through all physical devices to find the best one. */
	size_t choosen = 0;
	for (size_t i = 0, highscore = 0, score = 0; i < instance->GetPhysicalDeviceCount(); i++, score = 0)
	{
		/* Check if the physical device supports our framework. */
		const PhysicalDevice &physicalDevice = instance->GetPhysicalDevice(i);
		if (physicalDevice.SupportsPlutonium(surface))
		{
			/* Ask user if the current physical device is usable. */
			if (GpuPredicate(physicalDevice))
			{
				/* For more information about scoring see document 'Scoring'. */
				if (physicalDevice.GetType() == PhysicalDeviceType::DiscreteGpu) score += 6;
				if (physicalDevice.GetType() == PhysicalDeviceType::IntegratedGpu) score += 3;

				for (const QueueFamilyProperties &family : physicalDevice.GetQueueFamilies())
				{
					if (family.Flags == QueueFlag::Transfer) ++score;
					if (family.Flags == QueueFlag::Compute) ++score;
				}

				/* Set new highscore and update choosen physical device if needed. */
				if (score > highscore)
				{
					highscore = score;
					choosen = i;
				}
			}
		}
	}

	/* Return the physical device at the choosen index or simply the first if non passed the test. */
	return instance->GetPhysicalDevice(choosen);
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
	content = new AssetFetcher(*scheduler, *device);
	saver = new AssetSaver(*scheduler, *device);

	/* Window must be show at least once to give the correct size to the swapchain. */
	wnd->Show();
	gameWnd = new GameWindow(*wnd, *device);

	/* Move the log window out of the way on debug mode. */
#ifdef _DEBUG
	_CrtMoveDebugTerminal(*wnd);
#endif

	Initialize();
	
	/* Initialize and sort the components. */
	for (Component *cur : components) cur->DoInitialize();
	std::sort(components.begin(), components.end(), Component::SortPredicate);
	initialized = true;
}

void Pu::Application::DoFinalize(void)
{
	Finalize();

	for (Component *cur : components)
	{
		cur->Finalize();
		delete cur;
	}

	components.clear();

	delete content;
	delete saver;
	delete gameWnd;
	delete device;
	delete wnd;
	delete instance;
}

void Pu::Application::DoUpdate(float dt)
{
	/* Update all components that are set to update before the application update. */
	size_t i = 0;
	for (; i < components.size(); i++)
	{
		Component *cur = components[i];
		if (cur->place > 0) break;
		cur->Update(dt);
	}

	Update(dt);

	/* Update all components that are set to update after the application update. */
	for (; i < components.size(); i++) components[i]->Update(dt);
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