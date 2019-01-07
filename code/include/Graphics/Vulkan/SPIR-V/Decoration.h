#pragma once
#include <map>
#include "SPIRV.h"
#include "Core/String.h"

namespace Pu
{
	/* Defines a decoration attribute for a SPIR-V variable. */
	struct Decoration
	{
	public:
		/* Specifies the decorations found and their operands. */
		std::map<spv::Decoration, spv::Word> Numbers;

		/* Initializes an empty instance of a SPIR-V decoration. */
		Decoration(void)
		{}

		/* Merges the specifies decoration into this one. */
		inline void Merge(_In_ const Decoration &other)
		{
			/* Copy over all numbers. */
			for (const auto &[type, literal] : other.Numbers)
			{
				Numbers.emplace(type, literal);
			}
		}
	};
}