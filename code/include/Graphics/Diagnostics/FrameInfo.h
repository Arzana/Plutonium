#pragma once
#include "Graphics\GraphicsAdapter.h"

namespace Plutonium
{
	/* Saves the current values in the depth buffer as a greyscale image to a file (./debug/DepthInfo.png). */
	void _CrtSaveDepthToFile(_In_ GraphicsAdapter *device);
	/* Saves the current values in the stencil buffer as a greyscale image to a file (./debug/StencilInfo.png) */
	void _CrtSaveStencilToFile(_In_ GraphicsAdapter *device);
}