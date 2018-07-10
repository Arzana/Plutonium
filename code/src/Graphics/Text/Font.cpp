#define STB_TRUETYPE_IMPLEMENTATION

#include "Graphics\Text\Font.h"
#include "Core\Math\Basics.h"
#include "Core\SafeMemory.h"
#include "Core\Stopwatch.h"
#include "Core\StringFunctions.h"
#include "Streams\FileReader.h"
#include <stb\stb_truetype.h>
#include <cfloat>
#include <cstdio>
#include <locale>

using namespace Plutonium;

Plutonium::Font::~Font(void) noexcept
{
	free_s(chars);
	free_s(path);
}

Vector2 Plutonium::Font::MeasureString(const char * str) const
{
	if (!str) return Vector2::Zero;

	char32 *wstr = heapwstr(str);
	Vector2 result = MeasureString(wstr);
	free_s(wstr);
	return result;
}

Vector2 Plutonium::Font::MeasureString(const char32 * str) const
{
	if (!str) return Vector2::Zero;

	/* Initialize result and temporary values. */
	float width = 0.0f;
	float lineHeight = static_cast<float>(lineSpace);
	Vector2 offset = Vector2::Zero;
	bool firstChar = true;

	/* Loop through all chars in the string. */
	char32 c = *str;
	for (size_t i = 0; c != '\0'; c = str[++i])
	{
		/* Handle newlines. */
		if (c == '\r') continue;
		if (c == '\n')
		{
			lineHeight = static_cast<float>(lineSpace);
			offset.X = 0;
			offset.Y += static_cast<float>(lineSpace);
			firstChar = true;
			continue;
		}

		const Character *ch = GetCharOrDefault(static_cast<int32>(c));

		/* Make sure first characters cannot hang left of the bounding box with left side bearing. */
		if (firstChar)
		{
			offset.X = max(0.0f, ch->Bearing.X);
			firstChar = false;
		}

		/* Update width. */
		offset.X += static_cast<float>(ch->Advance);
		if (offset.X > width) width = offset.X;

		/* Update height. */
		if (ch->Size.Y > lineHeight) lineHeight = ch->Size.Y;
	}

	/* Return size. */
	return Vector2(width, offset.Y + lineHeight);
}

float Plutonium::Font::MeasureStringHeight(const char * str) const
{
	char32 *wstr = heapwstr(str);
	float result = MeasureStringHeight(wstr);
	free_s(wstr);
	return result;
}

float Plutonium::Font::MeasureStringHeight(const char32 * str) const
{
	/* Initialize result and temporary values. */
	float lineHeight = static_cast<float>(lineSpace);
	float y = 0;

	/* Loop through all chars in the string. */
	char32 c = *str;
	for (size_t i = 0; c != '\0'; c = str[++i])
	{
		/* Handle newlines. */
		if (c == '\r') continue;
		if (c == '\n')
		{
			lineHeight = static_cast<float>(lineSpace);
			y += static_cast<float>(lineSpace);
			continue;
		}

		const Character *ch = GetCharOrDefault(static_cast<int32>(c));

		/* Update height. */
		if (ch->Size.Y > lineHeight) lineHeight = ch->Size.Y;
	}

	return y + lineHeight;
}

const Character * Plutonium::Font::GetCharOrDefault(char32 key) const
{
	/* Search through storage for specified character. */
	for (size_t i = 0; i < cnt; i++)
	{
		Character *cur = chars + i;
		if (cur->Key == key) return cur;
	}

	/* Nothing found, return default. */
#if defined(DEBUG)
	static std::vector<char32> loggedKeys;
	if (std::find(loggedKeys.begin(), loggedKeys.end(), key) == loggedKeys.end())
	{
		loggedKeys.push_back(key);
		LOG_WAR("Character '%d' not found in font, replacing with '%c'!", key, chars[def].Key);
	}
#endif
	return chars + def;
}

Plutonium::Font::Font(void)
	: chars(nullptr), cnt(0), def(0), lineSpace(0), size(0)
{}

Font * Plutonium::Font::FromFile(const char * path, float size, WindowHandler wnd)
{
	/* Initialize result. */
	Font *result = new Font();
	FileReader reader(path, true);
	result->size = size;
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());

	/* Initialize file content buffer. */
	constexpr size_t TTF_BUFFER_SIZE = 1 << 20;
	byte *ttf_buffer = malloc_s(byte, TTF_BUFFER_SIZE);

	/* Load file into buffer. */
#if defined(_WIN32)
	FILE *file;
	if (!fopen_s(&file, path, "rb")) fread(ttf_buffer, 1, TTF_BUFFER_SIZE, file);
	else LOG_THROW("Unable to read binary font file!");
#else
	fread(ttf_buffer, 1, TTF_BUFFER_SIZE, fopen(path), "rb");
#endif

	/* Get global font info. */
	stbtt_fontinfo info;
	stbtt_InitFont(&info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));
	float scale = stbtt_ScaleForPixelHeight(&info, size);

	/* Initialize final character buffer. */
	result->cnt = info.numGlyphs;
	result->chars = malloc_s(Character, result->cnt);

	/* Log start and start stopwatch. */
	Stopwatch sw = Stopwatch::StartNew();

	/* Create character info and texture map. */
	result->SetCharacterInfo(&info, wnd, scale);
	result->PopulateTextureMap(&info, scale);

	/* Free file data and return result. */
	LOG("Finished initializing %d characters, took %Lf seconds.", result->cnt, sw.Seconds());
	free_s(ttf_buffer);
	return result;
}

void Plutonium::Font::SetCharacterInfo(stbtt_fontinfo * info, WindowHandler wnd, float scale)
{
	/* Get global rendering info. */
	int32 ascent, descent, lineGap;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

	/* Save the total size of the stitched texture map. */
	Vector2 finalMapSize = Vector2::Zero;
	Vector2 curLineSize = Vector2::Zero;

	/* Loop through character defines by the font. */
	int32 x = 0;
	for (int32 c = 0; c < cnt; c++, x++)
	{
		/* Get current character from buffer. */
		Character *cur = chars + c;

		/* Get character specific info. */
		int32 advance, lsb, x0, y0, x1, y1;
		stbtt_GetCodepointHMetrics(info, c, &advance, &lsb);
		stbtt_GetCodepointBitmapBoxSubpixel(info, c, scale, scale, 0, 0, &x0, &y0, &x1, &y1);

		/* Compute character boundsing box. */
		int32 w = x1 - x0, h = y1 - y0;

		/* Save known character details. */
		cur->Key = static_cast<char32>(c);
		cur->Size = Vector2(static_cast<float>(w), static_cast<float>(h));
		cur->Bounds = Rectangle(curLineSize.X, finalMapSize.Y, static_cast<float>(w), static_cast<float>(h));
		cur->Advance = static_cast<uint32>(rectify(x0 + advance * scale));
		cur->Bearing = Vector2(static_cast<float>(lsb * scale), static_cast<float>(y0));
		if (h > lineSpace) lineSpace = h;

		/* Update line size or map size. */
		if (x > 32)
		{
			/* Update final map size. */
			finalMapSize.X = max(finalMapSize.X, curLineSize.X);
			finalMapSize.Y += curLineSize.Y;

			/* Reset current line. */
			x = 0;
			curLineSize = Vector2::Zero;
		}
		else
		{
			/* Update current line. */
			curLineSize.X += w;
			curLineSize.Y = max(curLineSize.Y, static_cast<float>(h));
		}

		/* Make sure we set the default to a recognisable character. */
		if (cur->Key == U'?' && def == 0) def = static_cast<size_t>(c);
		if (cur->Key == U'\xFFFD') def = static_cast<size_t>(c);
	}

	/* Update to final map size. */
	finalMapSize.X = max(finalMapSize.X, curLineSize.X);
	finalMapSize.Y += curLineSize.Y;
	lineSpace += lineGap;
	map = new Texture(static_cast<int32>(finalMapSize.X), static_cast<int32>(finalMapSize.Y), wnd, &TextureCreationOptions::DefaultNoMipMap, "Fontmap");
}

void Plutonium::Font::PopulateTextureMap(stbtt_fontinfo * info, float scale)
{
	byte *data = malloc_s(byte, map->Width * map->Height * 4);
	Vector2 b2uv = Vector2::One / Vector2(static_cast<float>(map->Width), static_cast<float>(map->Height));

	/* Loop through character defines by the font. */
	int32 max = map->Width * map->Height;
	for (int32 c = 0; c < cnt; c++)
	{
		/* Get current character from buffer. */
		Character *cur = chars + c;

		/* Render to temporary alpha buffer. */
		int32 outw = ipart(cur->Size.X), outh = ipart(cur->Size.Y);
		if (outw < 1 || outh < 1) continue;

		byte *alphaBuffer = malloc_s(byte, outw * outh);
		stbtt_MakeCodepointBitmapSubpixel(info, alphaBuffer, outw, outh, outw, scale, scale, 0.0f, 0.0f, c);

		/* Populate data. */
		constexpr byte cv = static_cast<byte>(255);
		for (int32 y = 0; y < outh; y++)
		{
			for (int32 x = 0; x < outw; x++)
			{
				/* Get indices for source and destination buffer. */
				int32 i = y * outw + x;
				int32 j = static_cast<int32>((cur->Bounds.Position.Y + y) * map->Width + (cur->Bounds.Position.X + x)) << 2;
				byte cd = alphaBuffer[i] > 0 ? cv : 0;

				/* Set current pixel data. */
				data[j] = cd;
				data[j + 1] = cd;
				data[j + 2] = cd;
				data[j + 3] = alphaBuffer[i];
			}
		}

		/* Convert the bounds to a texture uv. */
		cur->Bounds.Position *= b2uv;
		cur->Bounds.Size *= b2uv;

		/* Free temporary buffers. */
		free_s(alphaBuffer);
	}

	/* Create map. */
	map->SetData(data);
	free_s(data);
}