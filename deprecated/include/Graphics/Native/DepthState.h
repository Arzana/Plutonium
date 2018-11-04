#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines states of depth testing. */
	enum class DepthState : GLenum
	{
		/* No depth testing. */
		None = 0,
#if defined(GL_NEVER)
		/* The fragment never passes the test. */
		Never = GL_NEVER,
#endif
#if defined (GL_ALWAYS)
		/* The fragment always passes the test. */
		Always = GL_ALWAYS,
#endif
#if defined(GL_LESS)
		/* Passes if the fragment is less than the buffered value. */
		Less = GL_LESS,
#endif
#if defined(GL_GREATER)
		/* Passes if the fragment is greater than the buffered value. */
		Greater = GL_GREATER,
#endif
#if defined(GL_EQUAL)
		/* Passes if the fragment is equal to the buffered value. */
		Equal = GL_EQUAL,
#endif
#if defined(GL_LEQUAL)
		/* Passes if the fragment is less or equal to the buffered value. */
		LessOrEqual = GL_LEQUAL,
#endif
#if defined(GL_GEQUAL)
		/* Passes if the fragment is greater or equal to the buffered value. */
		GreaterOrEqual = GL_GEQUAL,
#endif
#if defined(GL_NOTEQUAL)
		/* Passes if the fragment differs from the buffered value. */
		Differ = GL_NOTEQUAL
#endif
	};
}