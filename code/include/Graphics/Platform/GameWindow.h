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

		/* Gets the amount of command buffers specified. */
		_Check_return_ inline size_t GetCommandBufferCount(void) const
		{
			return buffers.size();
		}

		/* Gets the command buffer at the specified index. */
		_Check_return_ inline CommandBuffer& GetCommandBuffer(_In_ uint32 index)
		{
			return buffers.at(index);
		}

		/* Gets the image handle at the specified index. */
		_Check_return_ inline ImageHndl GetImage(_In_ uint32 index) const
		{
			return swapchain->GetImage(index);
		}

		void TestRun()
		{
			BeginRender();

			uint32 i = 0;
			for (CommandBuffer &buffer : buffers)
			{
				buffer.ClearImage(GetImage(i++), Color::Orange());
			}

			EndRender();
			SwapBuffers();
		}

	private:
		NativeWindow &native;
		LogicalDevice &device;
		Swapchain *swapchain;
		CommandPool *pool;
		uint32 queueIndex;
		vector<CommandBuffer> buffers;
		vector<Semaphore> semaphores;

		void SwapBuffers(void);
		void UpdateSwapchainSize(const NativeWindow&, ValueChangedEventArgs<Vector2> args);
		void CreateSwapchain(Extent2D size);
		void BeginRender(void);
		void EndRender(void);
	};
}