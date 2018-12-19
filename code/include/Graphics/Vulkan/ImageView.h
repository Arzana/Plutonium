#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a accessor object for images. */
	class ImageView
	{
	public:
		/* Initializes a new instance of an image view. */
		ImageView(_In_ LogicalDevice &device, _In_ const ImageViewCreateInfo &createInfo);
		ImageView(_In_ const ImageView&) = delete;
		/* Move constructor. */
		ImageView(_In_ ImageView &&value);
		/* Destroys the image view. */
		~ImageView(void)
		{
			Destroy();
		}

		_Check_return_ ImageView& operator =(_In_ const ImageView&) = delete;
		/* Move assignment. */
		_Check_return_ ImageView& operator =(_In_ ImageView &&other);

	private:
		friend class Framebuffer;

		ImageViewHndl hndl;
		LogicalDevice &parent;

		void Destroy(void);
	};
}