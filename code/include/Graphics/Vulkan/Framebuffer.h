#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	class ImageView;

	/* Defines a storage object for all images passed to a render pass. */
	class Framebuffer
	{
	public:
		Framebuffer(_In_ const Framebuffer&) = delete;
		/* Move constructor. */
		Framebuffer(_In_ Framebuffer &&value);
		/* Destroys the image view. */
		~Framebuffer(void)
		{
			Destroy();
		}

		_Check_return_ Framebuffer& operator =(_In_ const Framebuffer&) = delete;
		/* Move assignment. */
		_Check_return_ Framebuffer& operator =(_In_ Framebuffer &&other);

	private:
		FramebufferHndl hndl;
		LogicalDevice &parent;

		Framebuffer(LogicalDevice &device, RenderPassHndl renderPass, Extent2D dimensions, vector<ImageView*> attachments);

		void Destroy(void);
	};
}