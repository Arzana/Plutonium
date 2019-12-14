#pragma once
#include "Components/Component.h"
#include "Content/AssetFetcher.h"
#include "Content/AssetSaver.h"
#include "Input/InputDeviceHandler.h"
#include "Core/Diagnostics/Stopwatch.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	/* Defines a basic application loop object. */
	class Application
	{
	public:
		/* Whether ot not the update rate is fixed. */
		bool IsFixedTimeStep;

		/* Initializes a new instance of an application object. */
		Application(_In_ const wstring &name, _In_ float width, _In_ float height, _In_opt_ size_t threadCount = PuThread::GetMaxConcurrent() - 1);
		Application(_In_ const Application&) = delete;
		Application(_In_ Application&&) = delete;
		/* Releases the resources allocated by the application. */
		virtual ~Application(void);

		_Check_return_ Application& operator =(_In_ const Application&) = delete;
		_Check_return_ Application& operator =(_In_ Application&&) = delete;

		/* Starts the application and runs it untill the user closes it. */
		void Run(void);
		/* Adds a component to the application (Application takes ownership!). */
		void AddComponent(_In_ Component *component);
		/* Removes and deletes the specified component from the application. */
		void RemoveComponent(_In_ Component &component);

		/* Sets the target time setp in Hz. */
		inline void SetTargetTimeStep(_In_ int32 hertz)
		{
			targetElapTimeFocused = 1.0f / hertz;
			targetElapTimeBackground = min(targetElapTimeFocused, targetElapTimeBackground);
		}

		/* Gets the delta time used for the last render. */
		_Check_return_ inline float GetDeltaTime(void) const
		{
			return lastDt;
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

		/* Gets the asset fetcher. */
		_Check_return_ inline AssetFetcher& GetContent(void)
		{
			return *content;
		}

		/* Gets the asset saver. */
		_Check_return_ inline AssetSaver& GetSaver(void)
		{
			return *saver;
		}

		/* Gets the input handler. */
		_Check_return_ inline InputDeviceHandler& GetInput(void)
		{
			return *input;
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
			wnd->shouldSuppressRender = true;
		}

		/* Signals that the application should close. */
		inline void Exit(void)
		{
			wnd->Close();
			SuppressNextRender();
		}

		/* Returns whether the specified GPU can be choosen for the application. */
		virtual bool GpuPredicate(_In_ const PhysicalDevice& /* physicalDevice */) { return true; }
		/* Enalbes application specific physical device features that the application needs. */
		virtual void EnableFeatures(_In_ PhysicalDeviceFeatures& /* features */) { }
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
		/* Called after every render call. */
		virtual void PostRender(void) {}

	private:
		bool suppressUpdate, initialized;
		float prevTime, accumElapTime, maxElapTime, lastDt;
		float targetElapTimeFocused, targetElapTimeBackground;
		Stopwatch gameTime;
		uint32 graphicsQueueFamilyIndex;
		const wstring name;
		const Vector2 initialWndSize;

		VulkanInstance *instance;
		LogicalDevice *device;
		NativeWindow *wnd;
		GameWindow *gameWnd;
		TaskScheduler *scheduler;
		AssetFetcher *content;
		AssetSaver *saver;
		InputDeviceHandler *input;
		vector<Component*> components;

		static void InitializePlutonium(void);

		void InitializeVulkan(void);
		const PhysicalDevice& ChoosePhysicalDevice(void);
		bool Tick(void);
		void DoInitialize(void);
		void DoFinalize(void);
		void DoUpdate(float dt);
		void BeginRender(void);
		void DoRender(float dt);
		void EndRender(void);
	};
}