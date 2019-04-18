#pragma once
#include "GlyphType.h"
#include "Core/Math/Constants.h"
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines the arguments for glyph rejection. */
	struct GlyphRejectionArgs
		: public EventArgs
	{
		/* Specifies the glyph that has been rejected. */
		const char32 Glyph;
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

		/* Copy constructor. */
		GlyphRejectionArgs(_In_ const GlyphRejectionArgs &value)
			: EventArgs(), Glyph(value.Glyph), Type(value.Type),
			MaxLengthReached(value.MaxLengthReached), GlyphNotAllowed(value.GlyphNotAllowed),
			GlyphUsageOverriden(value.GlyphUsageOverriden)
		{}

		_Check_return_ GlyphRejectionArgs& operator =(_In_ const GlyphRejectionArgs&) = delete;
		_Check_return_ GlyphRejectionArgs& operator =(_In_ GlyphRejectionArgs&&) = delete;

		/* Gets a string representation of the most present error (result is a literal). */
		_Check_return_ const char* GetReason(void) const
		{
			if (GlyphUsageOverriden) return "The glyph is used for other purposes!";
			if (MaxLengthReached) return "The maximum length of the input field is reached!";
			if (GlyphNotAllowed) return "The type of glyph is not allowed in the input field!";
			return "No reason.";
		}
	};
}