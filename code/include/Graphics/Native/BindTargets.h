#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the platform specific buffer bind targets. */
	enum class BindTarget
	{
		/* This should never be used! */
		None = 0,
#if defined(GL_ARRAY_BUFFER)
		/* Used for vertex attributes. */
		Array = GL_ARRAY_BUFFER,
#endif
#if defined(GL_ATOMIC_COUNTER_BUFFER)
		/* Used for Atomic counter storage. */
		AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
#endif
#if defined(GL_COPY_READ_BUFFER)
		/* Used for buffer copy source. */
		CopyRead = GL_COPY_READ_BUFFER,
#endif
#if defined(GL_COPY_WRITE_BUFFER)
		/* Used for buffer copy destination. */
		CopyWrite = GL_COPY_WRITE_BUFFER,
#endif
#if defined(GL_DISPATCH_INDIRECT_BUFFER)
		/* Used for indirect compute dispatch commands. */
		DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
#endif
#if defined(GL_DRAW_INDIRECT_BUFFER)
		/* Used for indirect command arguments. */
		DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
#endif
#if defined(GL_ELEMENT_ARRAY_BUFFER)
		/* Used for vertex array indices. */
		ElementArray = GL_ELEMENT_ARRAY_BUFFER,
#endif
#if defined(GL_PIXEL_PACK_BUFFER)
		/* Used for pixel read target. */
		PixelPack = GL_PIXEL_PACK_BUFFER,
#endif
#if defined(GL_PIXEL_UNPACK_BUFFER)
		/* Used for texture data source. */
		PixelUnPack= GL_PIXEL_UNPACK_BUFFER,
#endif
#if defined(GL_QUERY_BUFFER)
		/* Used for query result buffer. */
		Query = GL_QUERY_BUFFER,
#endif
#if defined(GL_SHADER_STORAGE_BUFFER)
		/* Used for read and write storage for shaders. */
		ShaderStorage = GL_SHADER_STORAGE_BUFFER,
#endif
#if defined(GL_TEXTURE_BUFFER)
		/* Used for texture data buffer. */
		Texture = GL_TEXTURE_BUFFER,
#endif
#if defined(GL_TRANSFORM_FEEDBACK_BUFFER)
		/* Used for transform feedback buffer. */
		TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
#endif
#if defined(GL_UNIFORM_BUFFER)
		/* Used for uniform block storage. */
		Uniform = GL_UNIFORM_BUFFER
#endif
	};
}