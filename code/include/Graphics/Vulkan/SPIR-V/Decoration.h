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
		/* Specifies the member index (only used with uniform buffer members). */
		size_t MemberIndex;

		/* Initializes an empty instance of a SPIR-V decoration. */
		Decoration(void)
			: MemberIndex(0)
		{}

		/* Initializes a new instance of a SPIR-V decoration for a specific uniform buffer member. */
		Decoration(_In_ size_t memberIdx)
			: MemberIndex(memberIdx)
		{}

		/* Merges the specifies decoration into this one. */
		inline void Merge(_In_ const Decoration &other)
		{
			/* Copy over all numbers. */
			for (const auto &[type, literal] : other.Numbers)
			{
				Numbers.emplace(type, literal);
			}

			/* Make sure the member index is copied over correctly. */
			if (!MemberIndex && other.MemberIndex) MemberIndex = other.MemberIndex;
			else if (MemberIndex && other.MemberIndex) Log::Error("Unable to merge member index of decoration!");
		}
	};
}