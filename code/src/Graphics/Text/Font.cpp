#define STB_TRUETYPE_IMPLEMENTATION

#include "Graphics\Text\Font.h"
#include "Core\Math\Basics.h"
#include "Core\SafeMemory.h"
#include "Core\Stopwatch.h"
#include <stb\stb_truetype.h>
#include <cfloat>
#include <cstdio>
#include <locale>

using namespace Plutonium;

Plutonium::Font::~Font(void) noexcept
{
	free_s(chars);
}

Vector2 Plutonium::Font::MeasureString(const char * str) const
{
	/* Initialize result and allocate required temporary storage. */
	Vector2 result = Vector2(0.0f, FLT_MIN);
	int32 nlCnt = 0;

	char c = *str;
	Character *ch = chars + def;

	/* Loop through all chars in the string. */
	for (size_t i = 0; c != '\0'; i++, c = str[i])
	{
		ch = GetCharOrDefault(c);

		/* Update height if needed. */
		if (ch->Size.Y > result.Y) result.Y = ch->Size.Y;
		if (ch->Key == '\n') ++nlCnt;

		/* Update width. */
		result.X += (ch->Size.X - ch->Bearing.X) + ch->Advance;
	}

	/* Remove the last advance and add the newlines. */
	result.X -= ch->Advance;
	result.Y += static_cast<float>(nlCnt * lineSpace);

	return result;
}

Font * Plutonium::Font::FromFile(const char * path, float size)
{
	/* Initialize result. */
	Font *result = new Font();
	result->size = size;

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
	result->SetCharacterInfo(&info, scale);
	result->PopulateTextureMap(&info, scale);

	/* Free file data and return result. */
	LOG("Finished initializing %d characters, took %Lf seconds.", result->cnt, sw.Seconds());
	free_s(ttf_buffer);
	return result;
}

Plutonium::Font::Font(void)
	: chars(nullptr), cnt(0), def(0), lineSpace(0), size(0)
{}

void Plutonium::Font::SetCharacterInfo(stbtt_fontinfo * info, float scale)
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
		cur->Key = static_cast<char>(c);
		cur->Size = Vector2(static_cast<float>(w), static_cast<float>(h));
		cur->Bounds = Rectangle(curLineSize.X, finalMapSize.Y, static_cast<float>(w), static_cast<float>(h));
		cur->Advance = static_cast<uint32>(rectify(x0 + advance * scale));
		cur->Bearing = Vector2(static_cast<float>(lsb * scale), static_cast<float>(y0));
		if (h > lineSpace) lineSpace = h;

		/* Update line size or map size. */
		if (x > 32)
		{
			/* Update final map size. */
			finalMapSize.X = __max(finalMapSize.X, curLineSize.X);
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
		if (cur->Key == '?') def = static_cast<size_t>(c);
	}

	/* Update to final map size. */
	finalMapSize.X = max(finalMapSize.X, curLineSize.X);
	finalMapSize.Y += curLineSize.Y;
	lineSpace += lineGap;
	map = new Texture(static_cast<int32>(finalMapSize.X), static_cast<int32>(finalMapSize.Y), 0, "Fontmap");
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
		int32 outw = static_cast<int32>(cur->Size.X), outh = static_cast<int32>(cur->Size.Y);
		if (outw < 1 || outh < 1) continue;

		byte *alphaBuffer = malloc_s(byte, outw * outh);
		stbtt_MakeCodepointBitmapSubpixel(info, alphaBuffer, outw, outh, outw, scale, scale, 0.0f, 0.0f, c);

		/* Populate data. */
		byte cv = static_cast<byte>(255);
		for (int32 y = 0; y < outh; y++)
		{
			for (int32 x = 0; x < outw; x++)
			{
				/* Get indices for source and destination buffer. */
				int32 i = y * outw + x;
				int32 j = static_cast<int32>((cur->Bounds.Position.Y + y) * map->Width + (cur->Bounds.Position.X + x)) << 2;

				/* Set current pixel data. */
				data[j] = cv;
				data[j + 1] = cv;
				data[j + 2] = cv;
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

Character * Plutonium::Font::GetCharOrDefault(int32 key) const
{
	/* Search through storage for specified character. */
	for (size_t i = 0; i < cnt; i++)
	{
		Character *cur = chars + i;
		if (cur->Key == key) return cur;
	}

	/* Nothing found, return default. */
#if defined(DEBUG)
	LOG_WAR("Character %c not found in font, replacing with '%c'!", key, chars[def].Key);
#endif
	return chars + def;
}
