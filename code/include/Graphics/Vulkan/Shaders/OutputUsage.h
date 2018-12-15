#pragma once

namespace Pu
{
	/* Defines how the output field should be handled. */
	enum class OutputUsage
	{
		/* The output field should be handled as a color attachment, user has to set this value. */
		Color,
		/* The output field should be handled as a depth / stencil attachment, this is set be the pipeline. */
		DepthStencil,
		/* The output field should not be handled. */
		Preserve
	};
}