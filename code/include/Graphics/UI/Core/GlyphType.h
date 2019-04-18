#pragma once

namespace Pu
{
	/* Defines the types of categories a glyph can be in. */
	enum class GlyphType
	{
		/* A default alpha character (A-Z, CJK, etc.). */
		Character,
		/* A numberic character (0-9, etc.). */
		Number,
		/* A whitespace character ' ', '\t'. */
		Space,
		/* A punctuation mark (,. etc.). */
		Punctuation,
		/* A glyph that doesn't belong in any other category. */
		Special
	};
}