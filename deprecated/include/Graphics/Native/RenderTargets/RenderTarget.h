#pragma once
#include "RenderTargetAttachment.h"

namespace Plutonium
{
	class GraphicsAdapter;

	/* Defines a frame buffer object used to handle render targets. */
	class RenderTarget
	{
	public:
		/* Initializes a new render target with a specified width and height. */
		RenderTarget(_In_ GraphicsAdapter *device, _In_ bool attachDepthBuffer, _In_opt_ int32 height = 0, _In_opt_ int32 width = 0);
		RenderTarget(_In_ const RenderTarget &value) = delete;
		RenderTarget(_In_ RenderTarget &&value) = delete;
		/* Releases the resources allocated by the render target. */
		~RenderTarget(void);

		_Check_return_ RenderTarget& operator =(_In_ const RenderTarget &other) = delete;
		_Check_return_ RenderTarget& operator =(_In_ RenderTarget &&other) = delete;

		/* Adds a attachment of the specified type to this frame buffer. */
		_Check_return_ const RenderTargetAttachment* Attach(_In_ const char *name, _In_ AttachmentOutputType type, _In_opt_ bool autoBind = true);
		/* Finalizes this render target. */
		void Finalize(void);
		/* Copies the content of the render targets depth buffer to the default framebuffer. */
		void BlitDepth(void);
		/* Binds a specified attachment for writing. */
		void BindForWriting(_In_ const RenderTargetAttachment *attachment);

	private:
		friend class GraphicsAdapter;

		std::vector<RenderTargetAttachment*> attachments;
		size_t drawBufferCnt;
		uint32 ptrFbo, ptrRbo;
		GraphicsAdapter *device;
		int32 width, height;
		bool finalized;

		void AttachDepthBuffer(void);
	};
}