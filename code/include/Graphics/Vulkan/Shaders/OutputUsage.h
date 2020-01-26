#pragma once

namespace Pu
{
	/* Defines how the output field should be handled. */
	enum class OutputUsage
	{
		/* The usage of the output field is unknown at this point. */
		Unknown,
		/* The output field was a color render target in a previous subpass. */
		Input,
		/* The output field should be handled as a color attachment. */
		Color,
		/* The output field should be handled as a depth / stencil attachment. */
		DepthStencil,
		/* The output field should be resolved at the end of the pipeline. */
		Resolve,
		/* The output field should be preserved for a later subpass. */
		Preserve
	};

	/* Gets a human readable version of an output usage. */
	_Check_return_ inline const char* to_string(_In_ OutputUsage usage)
	{
		switch (usage)
		{
		case OutputUsage::Input:
			return "Input";
		case OutputUsage::Color:
			return "Color";
		case OutputUsage::DepthStencil:
			return "Depth/Stencil";
		case OutputUsage::Resolve:
			return "Resolve";
		case OutputUsage::Preserve:
			return "Preserve";
		default:
			return "Unknown";
		}
	}
}