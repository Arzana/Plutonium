#pragma once
#include "Core/Diagnostics/Stopwatch.h"
#include "Graphics/Platform/GameWindow.h"
#include "Core/Threading/Tasks/Scheduler.h"

namespace Pu
{
	/* Defines a basic application loop object. */
	class Application
	{
	public:
		/* Whether ot not the update rate is fixed. */
		bool IsFixedTimeStep;

		/* Initializes a new instance of an application object. */
		Application(_In_ const string &name);
		Application(_In_ const Application&) = delete;
		Application(_In_ Application&&) = delete;
		/* Releases the resources allocated by the application. */
		virtual ~Application(void);

		_Check_return_ Application& operator =(_In_ const Application&) = delete;
		_Check_return_ Application& operator =(_In_ Application&&) = delete;

		/* Starts the application and runs it untill the user closes it. */
		void Run(void);

		/* Sets the target time setp in Hz. */
		inline void SetTargetTimeStep(_In_ int32 hertz)
		{
			targetElapTimeFocused = 1.0f / hertz;
			targetElapTimeBackground = min(targetElapTimeFocused, targetElapTimeBackground);
		}

		inline void TempMarkDoneLoading(void)
		{
			loaded.store(true);
		}

		/* Gets the platform specific window. */
		_Check_return_ inline GameWindow& GetWindow(void)
		{
			return *gameWnd;
		}

		/* Gets the platform specific window. */
		_Check_return_ inline const GameWindow& GetWindow(void) const
		{
			return *gameWnd;
		}

		/* Gets the logical device for Vulkan graphics operations. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return *device;
		}

		/* Schedules the specified task for execution. */
		inline void ProcessTask(_In_ Task &task)
		{
			scheduler->Spawn(task);
		}

	protected:

		/* Supresses the next update call. */
		inline void SuppressNextUpdate(void)
		{
			suppressUpdate = true;
		}

		/* Supresses the next render call. */
		inline void SuppressNextRender(void)
		{
			suppressRender = true;
		}

		/* Signals that the application should close. */
		inline void Exit(void)
		{
			wnd->Close();
			suppressRender = true;
		}

		/* Returns whether the specified GPU can be choosen for the application. */
		virtual bool GpuPredicate(_In_ const PhysicalDevice& /* physicalDevice */) { return true; }
		/* Initializes the global objects needed for the application to run. */
		virtual void Initialize(void) = 0;
		/* Initializes the content objects needed for the application to run. */
		virtual void LoadContent(void) = 0;
		/* Finalizes the content objects loaded during LoadContent. */
		virtual void UnLoadContent(void) = 0;
		/* Finalizes the global objects initialized during Initialize. */
		virtual void Finalize(void) = 0;
		/* Updates the application. */
		virtual void Update(_In_ float dt) = 0;
		/* Called before every render call. */
		virtual void PreRender(void) {}
		/* Renders the application. */
		virtual void Render(_In_ float dt, _In_ CommandBuffer &cmdBuffer) = 0;
		/* Renders the appication (during load time). */
		virtual void RenderLoad(_In_ float dt) = 0;
		/* Called after every render call. */
		virtual void PostRender(void) {}

	private:
		bool suppressUpdate, suppressRender;
		float prevTime, accumElapTime, maxElapTime;
		float targetElapTimeFocused, targetElapTimeBackground;
		Stopwatch gameTime;
		uint32 graphicsQueueFamilyIndex;
		std::atomic_bool loaded;
		const string name;

		VulkanInstance *instance;
		LogicalDevice *device;
		NativeWindow *wnd;
		GameWindow *gameWnd;
		TaskScheduler *scheduler;

		static void InitializePlutonium(void);

		void InitializeVulkan(void);
		bool Tick(bool loading);
		void DoInitialize(void);
		void DoLoadContent(void);
		void DoFinalize(void);
		void DoUpdate(float dt, bool loading);
		void BeginRender(void);
		void DoRender(float dt);
		void DoRenderLoad(float dt);
		void EndRender(void);
	};
}