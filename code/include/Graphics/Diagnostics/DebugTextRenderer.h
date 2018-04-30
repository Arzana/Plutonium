#pragma once
#include "Graphics\Color.h"
#include "Graphics\Diagnostics\DebugRenderer.h"

namespace Plutonium
{
	struct FontRenderer;

	/* Provides ease of use debug text functionality. */
	struct DebugFontRenderer
		: DebugRenderer
	{
	public:
		/* Initializes a new instance of a debug font renderer. */
		DebugFontRenderer(_In_ Game *game, _In_ const char *font, _In_ const char *vrtxShdr, _In_ const char *fragShdr, _In_ int loadWeight, _In_opt_ Vector2 resetPos = Vector2::Zero, _In_opt_ Vector2 moveMod = Vector2::UnitY);
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

	protected:
		FontRenderer *renderer;

		virtual void Render(_In_ float dt) override;
		virtual void Finalize(void) override;
	};
}