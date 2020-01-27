#pragma once

namespace Pu
{
	/* Defines how the output field should be handled. */
	enum class OutputUsage
	{
		/* The usage of the output field is unknown at this point. */
		Unknown,
		/* The output field should be handled as a color attachment. */
		Color,
		/* The output field should be handled as a depth / stencil attachment. */
		DepthStencil,
		/* The output field should be resolved at the end of the pipeline. */
		Resolve
	};

	/* Gets a human readable version of an output usage. */
	_Check_return_ inline const char* to_string(_In_ OutputUsage usage)
	{
		switch (usage)
		{
		case OutputUsage::Color:
			return "Color";
		case OutputUsage::DepthStencil:
			return "Depth/Stencil";
		case OutputUsage::Resolve:
			return "Resolve";
		default:
			return "Unknown";
		}
	}
}