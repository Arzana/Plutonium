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

		/* Gets the name assigned to the font. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

	protected:
		const char *name;
		const char *path;

	private:
		friend struct FontRenderer;
		friend struct AssetLoader;

		Texture *map;
		Character *chars;
		size_t cnt, def;
		int32 lineSpace;
		float size;

		Font(void);

		static Font* FromFile(const char *path, float size, WindowHandler wnd);

		void SetCharacterInfo(stbtt_fontinfo *info, WindowHandler wnd, float scale);
		void PopulateTextureMap(stbtt_fontinfo *info, float scale);
		Character* GetCharOrDefault(int32 key) const;
	};
}