#pragma once
#include "NativeWindow.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Graphics/Vulkan/Swapchain.h"

namespace Pu
{
	class LogicalDevice;
	class PhysicalDevice;

	/* Defines a game ready window. */
	class GameWindow
	{
	public:
		/* Initializes a new instance of a game window from a native window on specified logical device. */
		GameWindow(_In_ NativeWindow &native, _In_ LogicalDevice &device);
		GameWindow(_In_ const GameWindow&) = delete;
		GameWindow(_In_ GameWindow&&) = delete;
		/* Releases the resources allocated by the window. */
		~GameWindow(void);

		_Check_return_ GameWindow& operator =(_In_ const GameWindow&) = delete;
		_Check_return_ GameWindow& operator =(_In_ GameWindow&&) = delete;

		/* Gets the current command buffer. */
		_Check_return_ inline CommandBuffer& GetCommandBuffer(void)
		{
			return buffers[curImgIdx];
		}

		void TestRun()
		{
			BeginRender();

			GetCommandBuffer().ClearImage(swapchain->GetImage(curImgIdx), Color::Orange());

			EndRender();
		}

	private:
		NativeWindow &native;
		LogicalDevice &device;
		Swapchain *swapchain;
		CommandPool *pool;
		uint32 queueIndex, curImgIdx;
		vector<CommandBuffer> buffers;
		vector<Semaphore> semaphores;

		void UpdateSwapchainSize(const NativeWindow&, ValueChangedEventArgs<Vector2> args);
		void CreateSwapchain(Extent2D size);
		void BeginRender(void);
		void EndRender(void);
	};
}