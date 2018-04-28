#pragma once
#include "Character.h"

struct stbtt_fontinfo;

namespace Plutonium
{
	/* Defines a font that can be used to render characters. */
	struct Font
	{
	public:
		Font(_In_ const Font &value) = delete;
		Font(_In_ Font &&value) = delete;
		/* Releases the resources allocated by the font. */
		~Font(void) noexcept;

		_Check_return_ Font& operator =(_In_ const Font &other) = delete;
		_Check_return_ Font& operator =(_In_ Font &&other) = delete;

		/* Measures the dimensions of the specified string as if it was rendered using this font. */
		_Check_return_ Vector2 MeasureString(_In_ const char *str) const;
		/* Loads a font from a specified file. */
		_Check_return_ static Font* FromFile(_In_ const char *path, _In_ float size, _In_ WindowHandler wnd);

		/* Gets the offset between lines. */
		_Check_return_ inline int32 GetLineSpace(void) const
		{
			return lineSpace;
		}
		/* gets the size of this font. */
		_Check_return_ inline float GetSize(void) const
		{
			return size;
		}

	private:
		friend struct FontRenderer;

		Texture *map;
		Character *chars;
		size_t cnt, def;
		int32 lineSpace;
		float size;

		Font(void);

		void SetCharacterInfo(stbtt_fontinfo *info, WindowHandler wnd, float scale);
		void PopulateTextureMap(stbtt_fontinfo *info, float scale);
		Character* GetCharOrDefault(int32 key) const;
	};
}