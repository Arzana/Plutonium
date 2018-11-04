#pragma once

namespace Plutonium
{
	/* Defines which chars are allowed to be added in the TextBox. */
	enum class InputFlags
	{
		/* No character are allowed. */
		NoChars = 1,
		/* No numbers are allowed. */
		NoNumbers = 2,
		/* No spaces are allowed. */
		NoSpaces = 4,
		/* No punctuation marks are allowed. */
		NoPunctuation = 8,
		/* No special character are allowed. */
		NoSpecials = 16,

		/* Everything may be added. */
		All = 0,
		/* Only basic text may be added. */
		Text = NoNumbers | NoSpecials,
		/* Only numbers can be added. */
		Numbers = NoChars | NoSpecials | NoSpaces | NoPunctuation
	};
}