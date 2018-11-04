#pragma once

namespace Plutonium
{
	/* Defines the types of categories a glyph can be in. */
	enum class GlyphType
	{
		/* Glyph type is not know. */
		Unknown,
		/* A default alpha character (A-Z, etc.). */
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