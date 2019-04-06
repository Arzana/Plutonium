#pragma once
#include "Graphics/Vulkan/ImageView.h"

namespace Pu
{
	class CommandBuffer;

	/* Defines a generic depth buffer image. */
	class DepthBuffer
		: public Image
	{
	public:
		/* Initializes a new instance of a 2D depth buffer image. */
		DepthBuffer(_In_ LogicalDevice &device, _In_ Format format, _In_ Extent2D size);
		DepthBuffer(_In_ const DepthBuffer&) = delete;
		/* Move constructor. */
		DepthBuffer(_In_ DepthBuffer &&value);
		/* Releases the resource allocated by the depth buffer. */
		virtual ~DepthBuffer(void)
		{
			Destroy();
		}

		_Check_return_ DepthBuffer& operator =(_In_ const DepthBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ DepthBuffer& operator =(_In_ DepthBuffer &&other);

		/* Makes the depth buffer ready for writing. */
		void MakeWritable(_In_ CommandBuffer &cmdBuffer);

		/* Gets the image view associated with the depth buffer. */
		_Check_return_ inline const ImageView& GetView(void) const
		{
			return *view;
		}

	private:
		ImageView *view;
		ImageAspectFlag aspect;

		ImageCreateInfo CreateImageInfo(Format depthFormat, Extent2D size);
		void SetAspect(Format depthFormat);

		void Destroy(void);
	};
}