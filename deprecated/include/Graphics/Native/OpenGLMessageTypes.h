#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the type of OpenGL messages. */
	enum class OpenGLMsgType : GLenum
	{
#if defined(GL_DEBUG_TYPE_ERROR)
		/* A basic OpenGL error. */
		Error = GL_DEBUG_TYPE_ERROR,
#endif
#if defined(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		/* Behavior marked as deprecated is used. */
		Deprecated = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
#endif
#if defined(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		/* Something triggered undefined behavior. */
		Undefined = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
#endif
#if defined(GL_DEBUG_TYPE_PORTABILITY)
		/* Relied upon functionality is not portable. */
		Portability = GL_DEBUG_TYPE_PORTABILITY,
#endif
#if defined(GL_DEBUG_TYPE_PERFORMANCE)
		/* Code creates possible performance issues. */
		Performance = GL_DEBUG_TYPE_PERFORMANCE,
#endif
#if defined(GL_DEBUG_TYPE_OTHER)
		/* Miscellaneous. */
		Other = GL_DEBUG_TYPE_OTHER
#endif
	};
}