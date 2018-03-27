#pragma once
#include "Graphics\Text\TextRenderer.h"

namespace Plutonium
{
	/* Provides ease of use debug text functionality. */
	struct DebugFontRenderer
		: public FontRenderer
	{
		/* Initializes a new instance of a debug font renderer. */
		DebugFontRenderer(_In_ GraphicsAdapter *device, _In_ const char *font, _In_ const char *vrtxShdr, _In_ const char *fragShdr, _In_opt_ Vector2 resetPos = Vector2::Zero);
		DebugFontRenderer(_In_ const DebugFontRenderer &value) = delete;
		DebugFontRenderer(_In_ DebugFontRenderer &&value) = delete;

		_Check_return_ DebugFontRenderer& operator =(_In_ const DebugFontRenderer &other) = delete;
		_Check_return_ DebugFontRenderer& operator =(_In_ DebugFontRenderer &&other) = delete;

		/* Adds a string to the font renderer to be rendered in the next frame at the debug text position. */
		inline void AddDebugString(_In_ const std::string &str, _In_ Color clr = Color::White)
		{
			AddDebugString(str.c_str(), clr);
		}
		/* Adds a string to the font renderer to be rendered in the next frame at the debug text position. */
		void AddDebugString(_In_ const char *str, _In_ Color clr = Color::White);
		/* Renders the strings specified throughout the frame. */
		virtual void Render(void);

		/* Gets the height of the debug menu. */
		_Check_return_ inline float GetDebugTextHeight(void) const
		{
			return defPos.Y + static_cast<float>(font->GetLineSpace());
		}

	private:
		const Vector2 reset;
		Vector2 defPos;
	};
}