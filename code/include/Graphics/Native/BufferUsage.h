#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the way buffered data will be accessed */
	enum class BufferUsage : GLenum
	{
#if defined(GL_STATIC_DRAW)
		/* The buffer is used for drawing, will be created once and used many times. */
		StaticDraw = GL_STATIC_DRAW,
#endif
#if defined(GL_STREAM_DRAW)
		/* The buffer is used for drawing, will be created once and used at most a few times. */
		StreamDraw = GL_STREAM_DRAW,
#endif
#if defined(GL_DYNAMIC_DRAW)
		/* The buffer is used for drawing, will be created many times and used many times. */
		DynamicDraw = GL_DYNAMIC_DRAW,
#endif
#if defined(GL_STATIC_READ)
		/* The buffer is used for reading data, will be created once and used many times. */
		StaticRead = GL_STATIC_READ,
#endif
#if defined(GL_STREAM_READ)
		/* The buffer is used for reading data, will be created once and used at most a few times. */
		StreamRead = GL_STREAM_READ,
#endif
#if defined(GL_DYNAMIC_READ)
		/* The buffer is used for reading data, will be created many times and used many times. */
		DynamicRead = GL_DYNAMIC_READ,
#endif
#if defined(GL_STATIC_COPY)
		/* The buffer is used as a source, will be created once and used many times. */
		StaticCopy = GL_STATIC_COPY,
#endif
#if defined(GL_STREAM_COPY)
		/* The buffer is used as a source, will be created once and used at most a few times. */
		StreamCopy = GL_STREAM_COPY,
#endif
#if defined(GL_DYNAMIC_COPY)
		/* The buffer is used as a source, will be created many times and used many times. */
		DynamicCopy = GL_DYNAMIC_COPY,
#endif
	};

	/* Gets the string variant of a BufferUsage value. */
	_Check_return_ inline const char* _CrtGetBufferUsageStr(_In_ BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::StaticDraw:
			return "Static draw";
		case BufferUsage::StreamDraw:
			return "Stream draw";
		case BufferUsage::DynamicDraw:
			return "Dynamic draw";
		case BufferUsage::StaticRead:
			return "Static read";
		case BufferUsage::StreamRead:
			return "Stream read";
		case BufferUsage::DynamicRead:
			return "Dynamic read";
		case BufferUsage::StaticCopy:
			return "Static copy";
		case BufferUsage::StreamCopy:
			return "Stream copy";
		case BufferUsage::DynamicCopy:
			return "Dynamic copy";
		default:
			return "Unknown";
		}
	}
}