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
		Resolve
	};

	/* Gets a human readable version of an output usage. */
	_Check_return_ inline const char* to_string(_In_ OutputUsage usage)
	{
		switch (usage)
		{
		case Pu::OutputUsage::Input:
			return "Input";
		case Pu::OutputUsage::Color:
			return "Color";
		case Pu::OutputUsage::DepthStencil:
			return "Depth/Stencil";
		case Pu::OutputUsage::Resolve:
			return "Resolve";
		case Pu::OutputUsage::Unknown:
		default:
			return "Unknown";
		}
	}
}