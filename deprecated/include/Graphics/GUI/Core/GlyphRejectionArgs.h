#pragma once
#include "Core\Events\EventArgs.h"
#include "GlyphType.h"

namespace Plutonium
{
	/* Defines the argument for a glyph rejection. */
	struct GlyphRejectionArgs
		: public EventArgs
	{
	public:
		/* Specifies the glyph that has been rejected. */
		const char32_t Glyph;
		/* The type of the glyph. */
		const GlyphType Type;
		/* Whether the rejection was because the maximum length has been reached. */
		const bool MaxLengthReached;
		/* Whether the rejection was because the specified glyph was not allowed in the text box. */
		const bool GlyphNotAllowed;
		/* Whether the rejection was because the glyph is used for other purposes. */
		const bool GlyphUsageOverriden;

		/* Initializes a new instance of the glyph rejection arguments object. */
		GlyphRejectionArgs(char32_t glyph, GlyphType type, bool maxLen, bool notAllowed, bool usageNa)
			: EventArgs(), Glyph(glyph), Type(type),
			MaxLengthReached(maxLen), GlyphNotAllowed(notAllowed), GlyphUsageOverriden(usageNa)
		{}

		/* Gets a string representation of the most present error. */
		_Check_return_ const char* GetReason(void) const
		{
			if (GlyphUsageOverriden) return "The glyph is used for other purposes!";
			if (MaxLengthReached) return "The maximum length of the input field is reached!";
			if (GlyphNotAllowed) return "The type of glyph is not allowed in the input field!";

			ASSERT("No reason for glyph rejection has been set!");
			return "";
		}
	};
}