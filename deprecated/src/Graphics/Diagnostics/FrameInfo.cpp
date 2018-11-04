#include "Graphics\Diagnostics\FrameInfo.h"

/* Defines the output directory for all debug images. */
#define OUTDIR(x)	"./debug/" x ".png"

using namespace Plutonium;

Texture* GenerateFrameTexture(GraphicsAdapter *device, const char *name)
{
	/* Get the width and height of the frame buffer. */
	const Plutonium::Rectangle vp = device->GetWindow()->GetClientBounds();
	int32 w = ipart(vp.GetWidth()), h = ipart(vp.GetHeight());

	/* Return frame texture. */
	return new Texture(w, h, device->GetWindow(), 0, name);
}

Texture * Plutonium::_CrtSaveDepthToTexture(GraphicsAdapter * device)
{
	/* Create texture buffer. */
	Texture *img = GenerateFrameTexture(device, "DepthInfo");
	size_t size = img->Width * img->Height;

	/* Get raw depth data from the frame buffer. */
	float *raw = malloca_s(float, size);
	glReadPixels(0, 0, img->Width, img->Height, GL_DEPTH_COMPONENT, GL_FLOAT, raw);

	/* Convert data to greyscale image. */
	byte *data = _CrtToGreyscale(raw, size, true, true);
	img->SetData(data);

	/* Free temporary buffers and return texture. */
	freea_s(raw);
	freea_s(data);
	return img;
}

Texture * Plutonium::_CrtSaveStencilToTexture(GraphicsAdapter * device)
{
	/* Create texture buffer. */
	Texture *img = GenerateFrameTexture(device, "StencilInfo");
	size_t size = img->Width * img->Height;

	/* Get raw stencil data from the frame buffer. */
	byte *raw = malloca_s(byte, size);
	glReadPixels(0, 0, img->Width, img->Height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, raw);

	/* Convert data to greyscale image. */
	byte *data = _CrtToGreyscale(raw, size);
	img->SetData(data);

	/* Free temporary buffers and return texture. */
	freea_s(raw);
	freea_s(data);
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