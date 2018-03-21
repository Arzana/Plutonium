#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines how the value in the stencil buffer should change when a test fails. */
	enum class StencilOperation : GLenum
	{
#if defined(GL_KEEP)
		/* Don't modify the current value. */
		Nothing = GL_KEEP,
#endif
#if defined(GL_ZERO)
		/* Sets the value to zero. */
		SetZero = GL_ZERO,
#endif
#if defined(GL_INCR)
		/* Increases the current value by one (max 255). */
		Increase = GL_INCR,
#endif
#if defined(GL_DECR)
		/* Decreases the current value one one (min 0). */
		Decrease = GL_DECR,
#endif
#if defined(GL_INCR_WRAP)
		/*  Increases the current value by one, wraping around if an overflow occurs. */
		IncreaseWrap = GL_INCR_WRAP,
#endif
#if defined(GL_DECR_WRAP)
		/* Decreases the current value by one, wraping around if an underflow occurs. */
		DecreaseWrap = GL_DECR_WRAP,
#endif
#if defined(GL_INVERT)
		/* Inverts the current value. */
		Invert = GL_INVERT,
#endif
#if defined(GL_REPLACE)
		/* Replaces the current value with the fragments value. */
		Replace = GL_REPLACE
#endif
	};
}