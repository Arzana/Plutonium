#pragma once
#include "Graphics/Vulkan/Shaders/Renderpass.h"

namespace Pu
{
	class ImageView;

	/* Defines a storage object for all images passed to a render pass. */
	class Framebuffer
	{
	public:
		/* Initializes a new instance of a framebuffer. */
		Framebuffer(_In_ LogicalDevice &device, _In_ const Renderpass &renderPass, _In_ Extent2D dimensions, _In_ const vector<const ImageView*> &attachments);
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

		/* Gets the full render area available for this framebuffer. */
		_Check_return_ inline const Rect2D& GetArea(void) const
		{
			return area;
		}

	private:
		friend class CommandBuffer;
		friend class Renderpass;

		FramebufferHndl hndl;
		Rect2D area;
		LogicalDevice &parent;

		void Destroy(void);
	};
}