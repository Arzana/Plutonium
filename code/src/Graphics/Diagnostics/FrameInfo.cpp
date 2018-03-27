#include "Graphics\Diagnostics\FrameInfo.h"
#include "Core\Math\Interpolation.h"

/* Defines the output directory for all debug images. */
#define OUTDIR(x)	"./debug/" x ".png"

using namespace Plutonium;

Texture* GenerateFrameTexture(GraphicsAdapter *device, const char *name)
{
	/* Get the width and height of the frame buffer. */
	const Plutonium::Rectangle vp = device->GetWindow()->GetClientBounds();
	int32 w = ipart(vp.GetWidth()), h = ipart(vp.GetHeight());

	/* Return frame texture. */
	return new Texture(w, h, 0, name);
}

template <typename _Ty>
void GetRange(_Ty *lowest, _Ty *highest, _Ty *buffer, size_t size, _Ty defl, _Ty defh)
{
	/* Set defaults. */
	*lowest = maxv<_Ty>();
	*highest = minv<_Ty>();

	/* Get range. */
	for (size_t i = 0; i < size; i++)
	{
		_Ty cur = buffer[i];
		if (cur != defl && cur < *lowest) *lowest = cur;
		if (cur != defh && cur > *highest) *highest = cur;
	}
}

template <typename _Ty>
void ToGreyscale(Texture *img, _Ty *rawData, bool invert, bool normalized)
{
	size_t size = img->Width * img->Height;

	/* Get value range. */
	_Ty lowest, highest;
	GetRange(&lowest, &highest, rawData, size, normalized ? static_cast<_Ty>(0) : maxv<_Ty>(), normalized ? static_cast<_Ty>(1) : minv<_Ty>());

	/* Convert to increase performance. */
	float c = static_cast<float>(lowest), d = static_cast<float>(highest);
	float a = (invert ? 255.0f : 0.0f), b = (invert ? 0.0f : 255.0f);

	LOG("Saved greyscale '%s' has range[%f-%f] -> [0-255].", img->GetName(), c, d);

	/* Convert raw data to greyscale (RGBA format). */
	byte *imgData = malloc_s(byte, size * 4);
	for (size_t i = 0, j = 0; i < size; i++)
	{
		/* Map raw data to byte range. */
		byte value = static_cast<byte>(map(a, b, static_cast<float>(rawData[i]), c, d));

		/* Convert mapped value to pre multiplied opaque color. */
		imgData[j++] = value;
		imgData[j++] = value;
		imgData[j++] = value;
		imgData[j++] = 255;
	}

	/* Send data to image and free temporary color buffer. */
	img->SetData(imgData);
	free_s(imgData);
}

Texture * Plutonium::_CrtSaveDepthToTexture(GraphicsAdapter * device)
{
	/* Create texture buffer. */
	Texture *img = GenerateFrameTexture(device, "DepthInfo");

	/* Get raw depth data from the frame buffer. */
	float *data = malloc_s(float, img->Width * img->Height);
	glReadPixels(0, 0, img->Width, img->Height, GL_DEPTH_COMPONENT, GL_FLOAT, data);

	/* Convert data to greyscale image. */
	ToGreyscale(img, data, true, true);
	free_s(data);

	return img;
}

Texture * Plutonium::_CrtSaveStencilToTexture(GraphicsAdapter * device)
{
	/* Create texture buffer. */
	Texture *img = GenerateFrameTexture(device, "StencilInfo");

	/* Get raw stencil data from the frame buffer. */
	byte *data = malloc_s(byte, img->Width * img->Height);
	glReadPixels(0, 0, img->Width, img->Height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, data);

	/* Convert data to greyscale image. */
	ToGreyscale(img, data, false, false);
	free_s(data);
	
	return img;
}

void Plutonium::_CrtSaveDepthToFile(GraphicsAdapter * device)
{
	/* Get texture and save it to the debug output. */
	Texture *img = _CrtSaveDepthToTexture(device);
	img->SaveAsPng(OUTDIR("DepthInfo"));
	delete_s(img);
}

void Plutonium::_CrtSaveStencilToFile(GraphicsAdapter * device)
{
	/* Get texture and save it to the debug output. */
	Texture *img = _CrtSaveStencilToTexture(device);
	img->SaveAsPng(OUTDIR("StencilInfo"));
	delete_s(img);
}