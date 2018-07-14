#pragma once
#include "RenderTargetAttachment.h"

namespace Plutonium
{
	struct GraphicsAdapter;

	/* Defines a frame buffer object used to handle render targets. */
	struct RenderTarget
	{
	public:
		/* Initializes a new render target with a specified width and height. */
		RenderTarget(_In_ GraphicsAdapter *device);
		RenderTarget(_In_ const RenderTarget &value) = delete;
		RenderTarget(_In_ RenderTarget &&value) = delete;
		/* Releases the resources allocated by the render target. */
		~RenderTarget(void);

		_Check_return_ RenderTarget& operator =(_In_ const RenderTarget &other) = delete;
		_Check_return_ RenderTarget& operator =(_In_ RenderTarget &&other) = delete;

		/* Adds a attachment of the specified type to this frame buffer. */
		_Check_return_ const RenderTargetAttachment* Attach(_In_ const char *name, _In_ AttachmentOutputType type);
		/* Finalizes this render target. */
		void Finalize(void);

	private:
		friend struct GraphicsAdapter;

		std::vector<RenderTargetAttachment*> attachments;
		uint32 ptr;
		GraphicsAdapter *device;
		int32 width, height;
	};
}