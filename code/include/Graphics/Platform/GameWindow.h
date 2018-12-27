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

		/* Creates a framebuffer for each swapchain image for the specific render pass with the specific image views (framebuffers are deleted if the window is resized!). */
		void CreateFrameBuffers(_In_ const Renderpass &renderPass, _In_ const vector<const ImageView*> &views);
		/* Gets the framebuffer associated with a specific render pass for the current swapchain image. */
		_Check_return_ const Framebuffer& GetCurrentFramebuffer(_In_ const Renderpass &renderPass) const;

		/* Gets the current command buffer. */
		_Check_return_ inline CommandBuffer& GetCommandBuffer(void)
		{
			return buffers[curImgIdx];
		}

		/* Gets the swapchain used by the game window. */
		_Check_return_ inline const Swapchain& GetSwapchain(void) const
		{
			return *swapchain;
		}

		/* Gets the current image used to present to the surface. */
		_Check_return_ inline ImageHndl GetCurrentImage(void) const
		{
			return swapchain->images[curImgIdx];
		}

		/* Gets the current image view used to present to the surface. */
		_Check_return_ inline const ImageView& GetCurrentImageView(void) const
		{
			return swapchain->views[curImgIdx];
		}

		/* Gets the native window used by the game window. */
		_Check_return_ inline NativeWindow& GetNative(void)
		{
			return native;
		}

		/* Gets the native window used by the game window. */
		_Check_return_ inline const NativeWindow& GetNative(void) const
		{
			return native;
		}

	private:
		friend class Application;

		NativeWindow &native;
		LogicalDevice &device;
		Swapchain *swapchain;
		CommandPool *pool;
		uint32 queueIndex, curImgIdx;

		vector<CommandBuffer> buffers;
		vector<Semaphore> semaphores;
		std::map<RenderPassHndl, vector<Framebuffer*>> frameBuffers;

		void OnNativeSizeChangedHandler(const NativeWindow&, ValueChangedEventArgs<Vector2> args);
		void CreateSwapchain(Extent2D size);
		void MakeSwapchainImageWritable(void);
		void MakeImagePresentable(void);
		void BeginRender(void);
		void EndRender(void);
		void DestroyFramebuffers(void);
	};
}