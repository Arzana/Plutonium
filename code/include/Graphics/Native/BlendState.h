#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines states of buffer blending.  */
	enum class BlendState
	{
		/* No blending is applied. */
		None = 0,
#if defined(GL_FUNC_ADD)
		/* The source and destination colors are added. */
		Additive = GL_FUNC_ADD,
#endif
#if defined(GL_FUNC_SUBTRACT)
		/* The destination is subtracted from the source color. */
		Subtractive = GL_FUNC_SUBTRACT,
#endif
#if defined(GL_FUNC_REVERSE_SUBTRACT)
		/* The source is subtracted from the destination color. */
		ReverseSubtractive = GL_FUNC_REVERSE_SUBTRACT,
#endif
#if defined(GL_MIN)
		/* The color is the result of a component-wise minimum comparison. */
		Minimum = GL_MIN,
#endif
#if defined(GL_MAX)
		/* The color is the result of a component-wise maximum comparison. */
		Maximum = GL_MAX
#endif
	};
}