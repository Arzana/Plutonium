#pragma once
#include "Glyph.h"
#include "Graphics/Textures/Texture2D.h"

namespace Pu
{
	/* Defines a font with a with a specific size. */
	class Font
		: public Asset
	{
	public:
		/* Initializes a new instance of a font from a specific file. */
		Font(_In_ LogicalDevice &device, _In_ const wstring &path);
		Font(_In_ const Font&) = delete;
		/* Move constructor. */
		Font(_In_ Font &&value);
		/* Releases the resources allocated by the font. */
		~Font(void)
		{
			Destroy();
		}

		_Check_return_ Font& operator =(_In_ const Font&) = delete;
		/* Move assignment. */
		_Check_return_ Font& operator =(_In_ Font &&other);

		/* Measures the dimensions of the specified string as if it was rendered using this font. */
		_Check_return_ Vector2 MeasureString(_In_ const string &str) const;
		/* Measures the dimensions of the specified string as if it was rendered using this font. */
		_Check_return_ Vector2 MeasureString(_In_ const wstring &str) const;
		/* Measures the dimensions of the specified string as if it was rendered using this font. */
		_Check_return_ Vector2 MeasureString(_In_ const ustring &str) const;
		/* Gets the glyph info of the specified character (or the default if it's not available in the font). */
		_Check_return_ const Glyph& GetGlyph(_In_ char32 key) const;

		/* Gets the offset between lines. */
		_Check_return_ inline int32 GetLineSpace(void) const
		{
			return lineSpace;
		}

		/* Gets the size of the font. */
		_Check_return_ inline float GetSize(void) const
		{
			return size;
		}

	private:
		Image *atlasImg;
		Texture2D *atlasTex;

		vector<Glyph> glyphs;
		size_t defaultGlyphIndex;
		int32 lineSpace;
		float size;

		void Destroy();
	};
}