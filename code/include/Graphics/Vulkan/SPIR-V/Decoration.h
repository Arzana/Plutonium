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
		/* The type of decoration. */
		spv::Decoration Type;
		/* The named literal numbers associated with the decoration. */
		std::map<string, spv::Word> Numbers;

		/* Initializes an empty instance of a SPIR-V decoration. */
		Decoration(void)
			: Type(spv::Decoration::Max)
		{}

		/* Initializes a new instance of a SPIR-V decoration. */
		Decoration(spv::Decoration type)
			: Type(type)
		{}
	};
}