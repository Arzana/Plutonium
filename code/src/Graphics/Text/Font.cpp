#define STB_TRUETYPE_IMPLEMENTATION

#include "Graphics\Text\Font.h"
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
	char c = *str;
	Character *ch = chars + def;

	/* Loop through all chars in the string. */
	for (size_t i = 0; c != '\0'; i++, c = str[i])
	{
		ch = GetCharOrDefault(c);
		
		/* Update height if needed. */
		if (ch->Size.Y > result.Y) result.Y = ch->Size.Y;
		result.X += (ch->Size.X - ch->Bearing.X) + ch->Advance;
	}

	/* Remove the last advance and return final measurement. */
	result.X -= ch->Advance;
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
	result->chars = malloc_s(Character, __min(UCHAR_MAX, result->cnt));

	/* Log start and start stopwatch. */
	LOG("Generating %d character textures.", __min(UCHAR_MAX, result->cnt));
	Stopwatch sw = Stopwatch::StartNew();

	/* Loop through character defines by the font. */
	for (int32 c = 0; c < result->cnt && c < UCHAR_MAX; c++)
	{
		/* Get current character from buffer. */
		Character *cur = result->chars + c;

		/* Get character specific info. */
		int32 advance, lsb, x0, y0, x1, y1;
		stbtt_GetCodepointHMetrics(&info, c, &advance, &lsb);
		stbtt_GetCodepointBitmapBoxSubpixel(&info, c, scale, scale, 0, 0, &x0, &y0, &x1, &y1);
		int32 w = x1 - x0, h = y1 - y0;

		/* Save known character details. */
		cur->Key = static_cast<char>(c);
		cur->Advance = static_cast<int32>(advance * scale);
		cur->Size = Vector2(static_cast<float>(w), static_cast<float>(h));
		cur->Bearing = Vector2(static_cast<float>(lsb >> 6), static_cast<float>(y0));
		if (h > result->lineSpace) result->lineSpace = h;

		/* Early out if the texture of the character would become to small. */
		if (w < 1 || h < 1)
		{
			cur->Texture = nullptr;
			continue;
		}

		/* Make sure we set the default to a recognisable character. */
		if (cur->Key == '?') result->def = static_cast<size_t>(c);

		/* Render to temporary alpha buffer. */
		byte *alphaBuffer = calloc_s(byte, w * h);
		stbtt_MakeCodepointBitmapSubpixel(&info, alphaBuffer, w, h, w, scale, scale, 0.0f, 0.0f, c);

		/* Create character texture without mip maps. */
		Texture *texture = new Texture(w, h, 0);
		byte *data = calloc_s(byte, w * h * 4);

		/* Populate data. */
		byte cv = static_cast<byte>(255);
		for (int32 y = 0; y < h; y++)
		{
			for (int32 x = 0; x < w; x++)
			{
				/* Get indices for both arrays. */
				int32 i = y * w + x;
				int32 j = i << 2;

				/* Set current pixel data. */
				data[j] = cv;
				data[j + 1] = cv;
				data[j + 2] = cv;
				data[j + 3] = alphaBuffer[i];
			}
		}

		/* Set character texture. */
		texture->SetData(data);
		cur->Texture = texture;

		/* Free temporary buffer.s */
		free_s(alphaBuffer);
		free_s(data);
	}

	/* Free file data and return result. */
	LOG("Finished initializing %d characters, took %Lf seconds.", result->cnt, sw.Seconds());
	free_s(ttf_buffer);
	return result;
}

Plutonium::Font::Font(void)
	: chars(nullptr), cnt(0), def(0), lineSpace(0), size(0)
{}

Character * Plutonium::Font::GetCharOrDefault(char key) const
{
	/* Search through storage for specified character. */
	for (size_t i = 0; i < cnt; i++)
	{
		Character *cur = chars + i;
		if (cur->Key == key) return cur;
	}

	/* Nothing found, return default. */
#if defined(DEBUG)
	LOG_WAR("Character %c not found in font, replacing with %c!", key, chars[def].Key);
#endif
	return chars + def;
}
