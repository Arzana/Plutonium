#pragma once

namespace Plutonium
{
	/* Defines the styles of displaying any filling style GuiItem. */
	enum class FillStyle
	{
		/* Bar will fill from left to right. */
		LeftToRight,
		/* Bar will fill from right to left. */
		RightToLeft,
		/* Bar will fill from top to bottom. */
		TopToBottom,
		/* Bar will fill from bottom to top. */
		BottomToTop
	};
}