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

constexpr size_t TTF_BUFFER_SIZE = 1 << 20;

Plutonium::Font::~Font(void) noexcept
{
	free_s(info);
	free_s(rawData);
	free_s(chars);
	free_s(path);
}

Vector2 Plutonium::Font::MeasureString(const char * str) const
{
	if (!str) return Vector2::Zero();

	char32 *wstr = heapwstr(str);
	Vector2 result = MeasureString(wstr);
	free_s(wstr);
	return result;
}

Vector2 Plutonium::Font::MeasureString(const char32 * str) const
{
	if (!str) return Vector2::Zero();

	/* Initialize result and temporary values. */
	float width = 0.0f;
	float lineHeight = static_cast<float>(lineSpace);
	Vector2 offset = Vector2::Zero();
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

float Plutonium::Font::GetKerning(char32 first, char32 second) const
{
	return static_cast<float>(stbtt_GetCodepointKernAdvance(info, first, second)) * scale;
}

Plutonium::Font::Font(void)
	: chars(nullptr), cnt(0), def(0), lineSpace(0), size(0)
{
	info = malloc_s(stbtt_fontinfo, 1);
	rawData = malloc_s(byte, TTF_BUFFER_SIZE);
}

Font * Plutonium::Font::FromFile(const char * path, float size, WindowHandler wnd)
{
	/* Initialize result. */
	Font *result = new Font();
	FileReader reader(path, true);
	result->size = size;
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());

	/* Load file into buffer. */
#if defined(_WIN32)
	FILE *file;
	if (!fopen_s(&file, path, "rb")) fread(result->rawData, 1, TTF_BUFFER_SIZE, file);
	else LOG_THROW("Unable to read binary font file!");
#else
	fread(ttf_buffer, 1, TTF_BUFFER_SIZE, fopen(path), "rb");
#endif

	/* Get global font info. */
	stbtt_InitFont(result->info, result->rawData, stbtt_GetFontOffsetForIndex(result->rawData, 0));
	result->scale = stbtt_ScaleForMappingEmToPixels(result->info, size);

	/* Initialize final character buffer. */
	result->cnt = result->info->numGlyphs;
	result->chars = malloc_s(Character, result->cnt);

	/* Log start and start stopwatch. */
	Stopwatch sw = Stopwatch::StartNew();

	/* Create character info and texture map. */
#if defined(DEBUG)
	size_t loaded = result->SetCharacterInfo(wnd);
#else
	result->SetCharacterInfo(wnd);
#endif
	result->PopulateTextureMap();

	/* Free file data and return result. */
	LOG("Finished initializing %zu/%zu characters, took %Lf seconds.", loaded, result->cnt, sw.Microseconds() * 0.000001L);
	return result;
}

size_t Plutonium::Font::SetCharacterInfo(WindowHandler wnd)
{
	/* Get global rendering info. */
	int32 ascent, descent, lineGap;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

	/* Save the total size of the stitched texture map. */
	Vector2 finalMapSize = Vector2::Zero();
	Vector2 curLineSize = Vector2::Zero();

	/* Loop through character defines by the font. */
	size_t i = 0;
	for (int32 glyph = 0, x = 0; i < cnt && glyph < maxv<uint16>(); glyph++)
	{
		/* Check if glyph is renderable. */
		if (stbtt_FindGlyphIndex(info, glyph) != 0)
		{
			/* Get current character from buffer. */
			Character *cur = chars + i;

			/* Get character specific info. */
			int32 advance, lsb, x0, y0, x1, y1;
			stbtt_GetCodepointHMetrics(info, glyph, &advance, &lsb);
			stbtt_GetCodepointBitmapBox(info, glyph, scale, scale, &x0, &y0, &x1, &y1);

			/* Compute character bounding box. */
			int32 w = x1 - x0, h = y1 - y0;

			/* Save known character details. */
			cur->Key = static_cast<char32>(glyph);
			cur->Size = Vector2(static_cast<float>(w), static_cast<float>(h));
			cur->Bounds = Rectangle(curLineSize.X, finalMapSize.Y, static_cast<float>(w), static_cast<float>(h));
			cur->Advance = static_cast<uint32>(rectify(x0 + advance * scale));
			cur->Bearing = Vector2(static_cast<float>(lsb) * scale, static_cast<float>(y0));
			if (h > lineSpace) lineSpace = h;

			/* Update line size or map size. */
			if (x > 32)
			{
				/* Update final map size. */
				finalMapSize.X = max(finalMapSize.X, curLineSize.X);
				finalMapSize.Y += curLineSize.Y;

				/* Reset current line. */
				x = 0;
				curLineSize = Vector2::Zero();
			}
			else
			{
				/* Update current line. */
				curLineSize.X += w;
				curLineSize.Y = max(curLineSize.Y, static_cast<float>(h));
			}

			/* Make sure we set the default to a recognisable character. */
			if ((cur->Key == U'?' && def == 0) || cur->Key == U'\xFFFD') def = i;

			++i;
			++x;
		}
	}

	/* Update to final map size. */
	finalMapSize.X = max(finalMapSize.X, curLineSize.X);
	finalMapSize.Y += curLineSize.Y;
	lineSpace += lineGap;
	map = new Texture(static_cast<int32>(finalMapSize.X), static_cast<int32>(finalMapSize.Y), wnd, &TextureCreationOptions::DefaultNoMipMap, "Fontmap");
	return i;
}

void Plutonium::Font::PopulateTextureMap(void)
{
	/* Allocate RGBA storage for font map, calloc is used to make sure we have no random noise between glyphs. */
	byte *data = calloc_s(byte, map->Width * map->Height * 4);
	Vector2 b2uv = Vector2::One() / Vector2(static_cast<float>(map->Width), static_cast<float>(map->Height));

	/* Loop through character defines by the font. */
	for (int32 c = 0; c < cnt; c++)
	{
		/* Get current character from buffer. */
		Character *cur = chars + c;

		/* Ignore glyphs that define no glyph face (space, tab, etc.). */
		int32 outw = ipart(cur->Size.X), outh = ipart(cur->Size.Y);
		if (outw < 1 || outh < 1) continue;

		/* Render to temporary alpha buffer. */
		byte *alphaBuffer = malloc_s(byte, outw * outh);
		stbtt_MakeCodepointBitmap(info, alphaBuffer, outw, outh, outw, scale, scale, cur->Key);

		/* Populate RGBA buffer with glyph alpha values. */
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

	/* Save fontmap as debug test. */
#if defined (DEBUG)
	constexpr const char *PREFIX = "Debug\\";
	constexpr const char *SUFFIX = "_FontMap.png";

	char *buffer = malloca_s(char, strlen(PREFIX) + strlen(name) + strlen(SUFFIX) + 1);
	const char *values[3] = { PREFIX, name, SUFFIX };

	mrgstr(values, 3, buffer);
	map->SaveAsPng(buffer, false);
	freea_s(buffer);
#endif
}