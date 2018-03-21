#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines how faces should be culled. */
	enum class FaceCullState : GLenum
	{
		/* No face culling. */
		None = 0,
#if defined(GL_FRONT)
		/* All front facing primitives will be culled. */
		Front = GL_FRONT,
#endif
#if defined(GL_BACK)
		/* All back facing primitives will be culled. */
		Back = GL_BACK,
#endif
#if defined(GL_FRONT_AND_BACK)
		/* All primitives will be culled. */
		All = GL_FRONT_AND_BACK
#endif
	};
}