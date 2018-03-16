#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the specific blend factors. */
	enum class BlendType
	{
#if defined(GL_ZERO)
		/* Color is disabled. */
		Zero = GL_ZERO,
#endif
#if defined(GL_ONE)
		/* Color is enabled. */
		One = GL_ONE,
#endif
#if defined(GL_ONE_MINUS_CONSTANT_COLOR)
		/* Each component is inverted. */
		Inverse = GL_ONE_MINUS_CONSTANT_COLOR,
#endif
#if defined(GL_SRC_COLOR)
		/* Each component is multiplied by the source color. */
		SrcColor = GL_SRC_COLOR,
#endif
#if defined(GL_ONE_MINUS_SRC_COLOR)
		/* Each component is multiplied by the inverse source color. */
		ISrcColor = GL_ONE_MINUS_SRC_COLOR,
#endif
#if defined(GL_SRC_ALPHA)
		/* Each component is multipled by the source alpha. */
		SrcAlpha = GL_SRC_ALPHA,
#endif
#if defined(GL_ONE_MINUS_SRC_ALPHA)
		/* Each component is multipled by the inverse source alpha. */
		ISrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
#endif
#if defined(GL_DST_COLOR)
		/* Each component is multiplied by the destination color. */
		DestColor = GL_DST_COLOR,
#endif
#if defined(GL_ONE_MINUS_DST_COLOR)
		/* Each component is multiplied by the inverse destination color. */
		IDestColor = GL_ONE_MINUS_DST_COLOR,
#endif
#if defined(GL_DST_ALPHA)
		/* Each component is multipled by the destination alpha. */
		DestAlpha = GL_DST_ALPHA,
#endif
#if defined(GL_ONE_MINUS_DST_ALPHA)
		/* Each component is multipled by the inverse destination alpha. */
		IDestAlpha = GL_ONE_MINUS_DST_ALPHA,
#endif
#if defined(GL_CONSTANT_ALPHA)
		/* Each component is replaced by the alpha. */
		Alpha = GL_CONSTANT_ALPHA,
#endif
#if defined(GL_ONE_MINUS_CONSTANT_ALPHA)
		/* Each component is replaced by the inverse alpha. */
		IAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
#endif
#if defined(GL_SRC_ALPHA_SATURATE)
		/* Source alpha will always be one. */
		SaturateAlpha = GL_SRC_ALPHA_SATURATE,
#endif
	};
}