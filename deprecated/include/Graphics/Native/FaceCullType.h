#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Determines which side is considered the front face. */
	enum class FaceCullType : GLenum
	{
#if defined(GL_CW)
		/* The front is determined by a clockwise algorithm. */
		ClockWise = GL_CW,
#endif
#if defined(GL_CCW)
		/* The front is determined by a counter clockwise algorithm. */
		CounterClockWise = GL_CCW
#endif
	};
}