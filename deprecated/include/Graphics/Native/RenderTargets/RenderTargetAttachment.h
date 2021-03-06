#pragma once
#include "Graphics\Texture.h"

namespace Plutonium
{
	/* Defines the types of interal formats used for a render target attachment. */
	enum class AttachmentOutputType : GLint
	{
		/* Attachment stores the depth (float). */
		LpDepth = GL_DEPTH_COMPONENT,
		/* Attachment stores the depth (float). */
		HpDepth = GL_DEPTH_COMPONENT32F,
		/* Attachment stores 3 unsigned bytes. */
		RGB = GL_RGB,
		/* Attachment stored 4 unsigned bytes. */
		RGBA = GL_RGBA,
		/* Attachment stores 3 low precision floats. */
		LpVector3 = GL_RGB16F,
		/* Attachment stores 4 low precision floats. */
		LpVector4 = GL_RGBA16F,
		/* Attachment stores 3 high precision floats. */
		HpVector3 = GL_RGB32F,
		/* Attachment stores 4 high precision floats. */
		HpVector4 = GL_RGBA32F
	};

	/* Defines a sampler based attachment for a render target. */
	class RenderTargetAttachment
	{
	public:
		RenderTargetAttachment(_In_ const RenderTargetAttachment &value) = delete;
		RenderTargetAttachment(_In_ RenderTargetAttachment &&value) = delete;
		/* Releases the resources allocated by the render target attachment. */
		~RenderTargetAttachment(void);

		_Check_return_ RenderTargetAttachment& operator =(_In_ const RenderTargetAttachment &other) = delete;
		_Check_return_ RenderTargetAttachment& operator =(_In_ RenderTargetAttachment &&other) = delete;

		/* Gets the assigned name of this render target attachment. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

		/* Gets the assigned type of this render target attachment. */
		_Check_return_ inline AttachmentOutputType GetType(void) const
		{
			return type;
		}

		/* Saves the tetxure as a specified file. */
		void SaveAsPng(_In_ const char *path, _In_opt_ bool flipVertically = true) const;

	private:
		friend class Uniform;
		friend class RenderTarget;

		const char *name;
		AttachmentOutputType type;
		int32 width, height;
		uint32 ptr;
		GLenum attachment;

#if defined (DEBUG)
		mutable bool bound;
#endif

		RenderTargetAttachment(const char *name, AttachmentOutputType type, size_t index, int32 width, int32 height);

		void Bind(void) const;
	};
}