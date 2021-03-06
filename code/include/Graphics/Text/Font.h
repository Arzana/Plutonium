#pragma once
#include "Glyph.h"
#include "CodeChart.h"
#include "Graphics/Textures/Texture2D.h"

struct stbtt_fontinfo;

namespace Pu
{
	/* Defines a font with a with a specific size. */
	class Font
		: public Asset
	{
	public:
		/* Initializes a new instance of a font from a specific file. */
		Font(_In_ float size, _In_ const CodeChart &codeChart);
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

		/* Gets the kerning advance between the first and second character. */
		_Check_return_ float GetKerning(_In_ char32 first, _In_ char32 second) const;
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

		/* Gets the combined image sampler for the font atlas. */
		_Check_return_ inline const Texture2D& GetAtlas(void) const
		{
			return *atlasTex;
		}

	protected:
		/* References the asset and returns itself. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

	private:
		friend class AssetLoader;
		friend class AssetFetcher;

		Image *atlasImg;
		Texture2D *atlasTex;
		stbtt_fontinfo *info;
		string data;

		CodeChart codeChart;
		vector<Glyph> glyphs;
		size_t defaultGlyphIndex;
		int32 lineSpace;
		float size;
		
		float GetScale(void) const;
		void Load(const wstring &path);
		Vector2 LoadGlyphInfo(void);
		void StageAtlas(Vector2 atlasSize, byte *dst);
		void Destroy();
	};
}