#include "Graphics/Text/Font.h"
#include "Streams/FileReader.h"

#ifdef _DEBUG
#include "Content/AssetSaver.h"
#endif

#define STB_TRUETYPE_IMPLEMENTATION

#include <stb/stb/stb_truetype.h>

Pu::Font::Font(LogicalDevice & device, const wstring & path, float size, const CodeChart &codeChart)
	: Asset(true, std::hash<wstring>{}(path)), device(device), size(size), codeChart(codeChart)
{
	info = new stbtt_fontinfo();
	Load(path, false);
}

Pu::Font::Font(Font && value)
	: Asset(std::move(value)), atlasImg(value.atlasImg), atlasTex(value.atlasTex),
	glyphs(std::move(value.glyphs)), defaultGlyphIndex(value.defaultGlyphIndex),
	lineSpace(value.lineSpace), size(value.size), info(value.info), device(value.device),
	data(std::move(value.data)), codeChart(std::move(value.codeChart))
{
	value.atlasImg = nullptr;
	value.atlasTex = nullptr;
	value.info = nullptr;
}

Pu::Font & Pu::Font::operator=(Font && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		device = std::move(other.device);
		atlasImg = other.atlasImg;
		atlasTex = other.atlasTex;
		glyphs = std::move(glyphs);
		codeChart = std::move(other.codeChart);
		defaultGlyphIndex = other.defaultGlyphIndex;
		lineSpace = other.lineSpace;
		size = other.size;
		info = other.info;
		data = std::move(other.data);

		other.atlasImg = nullptr;
		other.atlasTex = nullptr;
		other.info = nullptr;
	}

	return *this;
}

Pu::Vector2 Pu::Font::MeasureString(const ustring & str) const
{
	/* Early out. */
	if (str.empty()) return Vector2();
	const float lineHeight = static_cast<float>(lineSpace);

	/* Initializes the values used within the loop. */
	float width = 0.0f;
	Vector2 offset(0.0f, lineHeight);

	/* Loop through all glyphs. */
	for (const char32 c : str)
	{
		/* Handle newline. */
		if (c == U'\r') continue;
		if (c == U'\n')
		{
			width = max(width, offset.X);
			offset.Y += lineHeight;

			offset.X = 0.0f;
			continue;
		}

		/* Add the advance to the character. */
		offset.X += static_cast<float>(GetGlyph(c).Advance);
	}

	return Vector2(max(width, offset.X), offset.Y);
}

const Pu::Glyph & Pu::Font::GetGlyph(char32 key) const
{
	for (const Glyph &cur : glyphs)
	{
		if (cur.Key == key) return cur;
	}

	return glyphs[defaultGlyphIndex];
}

Pu::Asset & Pu::Font::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

float Pu::Font::GetScale(void) const
{
	return stbtt_ScaleForMappingEmToPixels(info, size);
}

void Pu::Font::Load(const wstring & path, bool viaLoader)
{
	/* Read the byte data from the file. */
	FileReader reader(path);
	data = reader.ReadToEnd();

	/* Initialize the global font information. */
	stbtt_InitFont(info, reinterpret_cast<unsigned char*>(data.data()), 0);
	glyphs.resize(min(codeChart.GetCharacterCount(), static_cast<size_t>(info->numGlyphs)));

	/* Load the glyphs defined in the font. */
	const float scale = GetScale();
	const Vector2 requiredImgSize = LoadGlyphInfo(scale);

	/* 
	We call difference functions for the atlas creation.
	If it's done via the loader we create a task for it so we can be optimal.
	For the direct loading we just stall this thread to wait for the command to complete.
	*/
	if (viaLoader); //TODO implement loader.
	else CreateAtlas(requiredImgSize, scale);
}

Pu::Vector2 Pu::Font::LoadGlyphInfo(float scale)
{
	/* Get the global vertical information of the font. */
	int32 ascent, descent;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineSpace);

	/* Set the font information for all characters in the font or up until the UTF-16 limit. */
	Vector2 finalImgSize, curLineSize;
	size_t i = 0, j = 0;
	for (char32 key : codeChart)
	{
		/* Make sure the character is represented in the font. */
		if (stbtt_FindGlyphIndex(info, key))
		{
			/* Get the glyph information. */
			int32 advance, lsb, x0, x1, y0, y1;
			stbtt_GetCodepointHMetrics(info, key, &advance, &lsb);
			stbtt_GetCodepointBitmapBox(info, key, scale, scale, &x0, &y0, &x1, &y1);

			/* Convert the raw information to the glyph structure. */
			Glyph &cur = glyphs[i];
			cur.Key = key;
			cur.Size = Vector2(static_cast<float>(x1 - x0), static_cast<float>(y1 - y0));
			cur.Bounds = Rectangle(curLineSize.X, finalImgSize.Y, cur.Size.X, cur.Size.Y);
			cur.Advance = static_cast<uint32>(rectify(x0 + advance * scale));
			cur.Bearing = Vector2(static_cast<float>(lsb), static_cast<float>(y0));

			/* Create a new line in the font map after every 32 glyphs. */
			if (j > 32)
			{
				/* Update the image size. */
				finalImgSize.X = max(finalImgSize.X, curLineSize.X);
				finalImgSize.Y += curLineSize.Y;

				/* Reset the curent line. */
				j = 0;
				curLineSize = Vector2();
			}
			else
			{
				curLineSize.X += cur.Size.X;
				curLineSize.Y = max(curLineSize.Y, cur.Size.Y);
			}

			/* Set the default glyph to either the unicode standart or the question mark. */
			if (key == U'\xFFFD') defaultGlyphIndex = i;
			else if (key == U'?' && !defaultGlyphIndex) defaultGlyphIndex = i;

			i++;
			j++;
		}
	}

	/* Release all the memory we didn't need in the end. */
	Log::Verbose("Loaded font with %zu/%zu/%d characters.", i, glyphs.size(), info->numGlyphs);
	glyphs.erase(glyphs.begin() + i, glyphs.end());

	/*  Finalize the image size and return. */
	finalImgSize.X = max(finalImgSize.X, curLineSize.X);
	finalImgSize.Y += curLineSize.Y;
	return finalImgSize;
}

void Pu::Font::CreateAtlas(Vector2 size, float scale)
{
	/* Define the size of the staging buffer and create a converter from the bounds to the final uv coords. */
	const size_t atlasSizeBytes = static_cast<size_t>(size.X) * static_cast<size_t>(size.Y);
	const Vector2 b2uv = Vector2(1.0f) / size;
	const int stride = ipart(size.X);

	/* Create the staging buffer and get the CPU/GPU visible memory pointer. */
	StagingBuffer src(device, atlasSizeBytes);
	src.BeginMemoryTransfer();
	byte *buffer = reinterpret_cast<byte*>(src.GetHostMemory());

	for (Glyph &cur : glyphs)
	{
		/* Ignore rendering glyphs that have no image data (space, tab, etc.). */
		if (cur.Size.X < 1.0f || cur.Size.Y < 1.0f) continue;

		/* Render the codepoint alpha to the result buffer. */
		const size_t x = static_cast<size_t>(cur.Bounds.Position.X);
		const size_t y = static_cast<size_t>(cur.Bounds.Position.Y);
		const size_t offset = y * stride + x;
		stbtt_MakeCodepointBitmap(info, buffer + offset, ipart(cur.Size.X), ipart(cur.Size.Y), stride, scale, scale, cur.Key);

		/* Convert the bouds to texture coordinates. */
		cur.Bounds.Position *= b2uv;
		cur.Bounds.Size *= b2uv;
	}

	/* Save the fontmap on debug for testing TODO add name to this. */
#ifdef _DEBUG
	AssetSaver::SaveImageInternal(buffer, ipart(size.X), ipart(size.Y), 1, L"Debug\\FontMap", ImageSaveFormats::Png);
#endif

	src.EndMemoryTransfer();
}

void Pu::Font::Destroy()
{
	if (atlasImg) delete atlasImg;
	if (atlasTex) delete atlasTex;
	if (info) delete info;
}