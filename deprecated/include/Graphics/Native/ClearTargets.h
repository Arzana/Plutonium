#pragma once
#include <glad\glad.h>
#include "Core\EnumUtils.h"

namespace Plutonium
{
	/* Defines the target for a clear buffer command. */
	enum class ClearTarget : GLbitfield
	{
#if defined (GL_COLOR_BUFFER_BIT)
		/* The color buffer. */
		Color = GL_COLOR_BUFFER_BIT,
#endif
#if defined (GL_DEPTH_BUFFER_BIT)
		/* The depth (or z) buffer. */
		Depth = GL_DEPTH_BUFFER_BIT, 
#endif
#if defined (GL_STENCIL_BUFFER_BIT)
		/* The stencil buffer. */
		Stencil = GL_STENCIL_BUFFER_BIT,
#endif
#if defined (GL_ACCUM_BUFFER_BIT)
		/* The accumulation buffer. */
		Accumulation = GL_ACCUM_BUFFER_BIT
#endif
	};

	/* Combines the clear targets. */
	_Check_return_ inline constexpr ClearTarget operator |(ClearTarget first, ClearTarget second)
	{
		return _CrtEnumBitOr(first, second);
	}
}