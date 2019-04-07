#pragma once

namespace Pu
{
	/* Defines the available output formats for an image save to disk operation. */
	enum class ImageSaveFormats
	{
		/* The PNG compressed file format. */
		Png,
		/* The Bitmap uncompressed file format. */
		Bmp,
		/* The TGA compressed file format. */
		Tga,
		/* The JPG compressed file format, optionally a quality value can be passed with range: [0, 100]. */
		Jpg,
		/* The HDR uncompressed file format. */
		Hdr
	};
}