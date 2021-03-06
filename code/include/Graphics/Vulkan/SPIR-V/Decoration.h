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
		/* Specifies any decorations without extra operands. */
		vector<spv::Decoration> Flags;
		/* Specifies the decorations found and their operands. */
		std::map<spv::Decoration, spv::Word> Numbers;
		/* Specifies the member offset (in bytes) (only used with uniform buffer members). */
		size_t MemberOffset;

		/* Initializes an empty instance of a SPIR-V decoration. */
		Decoration(void)
			: MemberOffset(0)
		{}

		/* Initializes a new instance of a SPIR-V decoration for a specific uniform buffer member. */
		Decoration(_In_ size_t offset)
			: MemberOffset(offset)
		{}

		/* Gets whether the specified SPIR-V decoration is present. */
		inline bool Contains(_In_ spv::Decoration key) const
		{
			if (Flags.contains(key)) return true;
			return Numbers.find(key) != Numbers.end();
		}

		/* Merges the specifies decoration into this one. */
		inline void Merge(_In_ const Decoration &other)
		{
			Flags.insert(Flags.end(), other.Flags.begin(), other.Flags.end());

			/* Copy over all numbers. */
			for (const auto &[type, literal] : other.Numbers)
			{
				Numbers.emplace(type, literal);
			}
		}
	};
}