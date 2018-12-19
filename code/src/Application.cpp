#include "Application.h"
#include "Core/Threading/ThreadUtils.h"
#include "Graphics/Vulkan/Instance.h"
#include "Core/EnumUtils.h"
#include "Core/Diagnostics/DbgUtils.h"

#ifdef _WIN32
#include "Graphics/Platform/Windows/Win32Window.h"
#endif

Pu::Application::Application(const string & name)
	: IsFixedTimeStep(true), suppressUpdate(false), suppressRender(false), name(name),
	targetElapTimeFocused(1.0f / 60.0f), targetElapTimeBackground(1.0f / 20.0f),
	maxElapTime(5.0f), accumElapTime(0.0f), gameTime(), device(nullptr), loaded(false)
{
	InitializePlutonium();
	scheduler = new TaskScheduler();
}

Pu::Application::~Application(void)
{
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
	const bool oldFixed = IsFixedTimeStep;
	DoLoadContent();
	prevTime = gameTime.SecondsAccurate();
	while (!loaded.load())
	{
		/* Check if used wants to close the window during load time. */
		if (wnd->Update()) return;

		/* Tick application. */
		while (!Tick(true));
	}

	Log::Verbose("Finished initializing and loading content for '%s', took %f seconds.", wnd->GetTitle(), gameTime.SecondsAccurate());
	IsFixedTimeStep = oldFixed;

	/* Run application loop. */
	while (wnd->Update())
	{
		while (!Tick(false));
	}

	/* Finalize application. */
	UnLoadContent();
	DoFinalize();
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
	_CrtSetCurrentThreadName("PuMain");
}

void Pu::Application::InitializeVulkan(void)
{
	/* Create the Vulkan instance, ew need the surface extensions for the native window. */
	instance = new VulkanInstance(name.c_str(),
		{
			u8"VK_KHR_surface",
#ifdef _WIN32
			u8"VK_KHR_win32_surface"
#endif
		});

	/* Create the native window. */
#ifdef _WIN32
	wnd = new Win32Window(*instance, name.c_str(), Vector2(600.0f));
#else
	Log::Fatal("Unable to create window on this platform!");
#endif

	/* Get all physical devices avialable. */
	vector<int32> graphicsQueues(instance->GetPhysicalDeviceCount(), -1);

	/* Check which physical devices are compatible with Plutonium and have a graphics queue. */
	for (size_t i = 0; i < instance->GetPhysicalDeviceCount(); i++)
	{
		const PhysicalDevice &physicalDevice = instance->GetPhysicalDevice(i);

		if (physicalDevice.SupportsPlutonium())
		{
			const vector<QueueFamilyProperties> queueFamilies = physicalDevice.GetQueueFamilies();
			for (uint32 j = 0; j < queueFamilies.size(); j++)
			{
				//TODO: maybe add specific compute and transfer queue to speed up those things.
				if (_CrtEnumCheckFlag(queueFamilies[j].Flags, QueueFlag::Graphics) &&
					wnd->GetSurface().QueueFamilySupportsPresenting(j, physicalDevice))
				{
					graphicsQueues[i] = j;
					break;
				}
			}
		}
	}

	/* Let the user choose from the acceptable physical devices. */
	for (size_t i = 0; i < instance->GetPhysicalDeviceCount(); i++)
	{
		const int32 graphicsQueueIdx = graphicsQueues[i];
		const PhysicalDevice &physicalDevice = instance->GetPhysicalDevice(i);

		/* A graphics queue needs to be available. */
		if (graphicsQueueIdx == -1) continue;

		/* Ask user if this physical device is oke. */
		if (GpuPredicate(physicalDevice))
		{
			constexpr float priorities[1] = { 1.0f };
			constexpr const char *deviceExtensions[1] = { u8"VK_KHR_swapchain" };

			/* Create logical device. */
			DeviceQueueCreateInfo queueInfo(static_cast<uint32>(graphicsQueueIdx), 1, priorities);
			DeviceCreateInfo deviceInfo(1, &queueInfo, 1, deviceExtensions);
			device = physicalDevice.CreateLogicalDevice(&deviceInfo);

			break;
		}
	}

	/* Log if no physical device was choosen for operations. */
	if (!device) Log::Fatal("Unable to create Vulkan logical device!");
}

bool Pu::Application::Tick(bool loading)
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
				DoUpdate(targetElapTime, loading);
			}
		}
		else
		{
			/* Do updates. */
			dt = accumElapTime;
			accumElapTime = 0.0f;
			DoUpdate(dt, loading);
		}
	}

	/* Do frame render. */
	if (suppressRender) suppressRender = false;
	else if (loading) DoRenderLoad(dt);
	else DoRender(dt);

	return true;
}

void Pu::Application::DoInitialize(void)
{
	InitializeVulkan();

	/* Window must be show at least once to give the correct size to the swapchain. */
	wnd->Show();
	gameWnd = new GameWindow(*wnd, *device);

	/* Move the log window out of the way on debug mode. */
#ifdef _DEBUG
	_CrtMoveDebugTerminal(*wnd);
#endif

	Initialize();
}

void Pu::Application::DoLoadContent(void)
{
	IsFixedTimeStep = false;
	LoadContent();
}

void Pu::Application::DoFinalize(void)
{
	Finalize();

	delete gameWnd;
	delete device;
	delete wnd;
	delete instance;
}

void Pu::Application::DoUpdate(float dt, bool loading)
{
	if (!loading) Update(dt);
}

void Pu::Application::BeginRender(void)
{
	/* Start the current command buffer and call prerender. */
	gameWnd->BeginRender();
	PreRender();
}

void Pu::Application::DoRender(float dt)
{
	BeginRender();
	Render(dt, gameWnd->GetCommandBuffer());
	EndRender();
}

void Pu::Application::DoRenderLoad(float dt)
{
	BeginRender();
	RenderLoad(dt);
	EndRender();
}

void Pu::Application::EndRender(void)
{
	PostRender();
	gameWnd->EndRender();
}