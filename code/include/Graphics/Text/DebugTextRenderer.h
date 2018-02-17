#pragma once
#include "Graphics\Text\TextRenderer.h"

namespace Plutonium
{
	/* Provides ease of use debug text functionality. */
	struct DebugFontRenderer
		: public FontRenderer
	{
		/* Initializes a new instance of a debug font renderer. */
		DebugFontRenderer(_In_ WindowHandler wnd, _In_ const char *font, _In_ const char *vrtxShdr, _In_ const char *fragShdr);
		DebugFontRenderer(_In_ const DebugFontRenderer &value) = delete;
		DebugFontRenderer(_In_ DebugFontRenderer &&value) = delete;

		_Check_return_ DebugFontRenderer& operator =(_In_ const DebugFontRenderer &other) = delete;
		_Check_return_ DebugFontRenderer& operator =(_In_ DebugFontRenderer &&other) = delete;

		/* Adds a string to the font renderer to be rendered in the next frame at the debug text position. */
		void AddDebugString(_In_ const char *str);
		/* Renders the strings specified throughout the frame. */
		virtual void Render(void);

	private:
		Vector2 defPos;
	};
}