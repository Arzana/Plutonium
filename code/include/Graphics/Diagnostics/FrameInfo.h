#pragma once
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Texture.h"
#include "Core\Math\Interpolation.h"

namespace Plutonium
{
	/* Saves the current values in the depth buffer as a greyscale image to a texture. */
	_Check_return_ Texture* _CrtSaveDepthToTexture(_In_ GraphicsAdapter *device);
	/* Saves the current values in the stencil buffer as a greyscale image to a texture. */
	_Check_return_ Texture* _CrtSaveStencilToTexture(_In_ GraphicsAdapter *device);

	/* Saves the current values in the depth buffer as a greyscale image to a file (./debug/DepthInfo.png). */
	void _CrtSaveDepthToFile(_In_ GraphicsAdapter *device);
	/* Saves the current values in the stencil buffer as a greyscale image to a file (./debug/StencilInfo.png). */
	void _CrtSaveStencilToFile(_In_ GraphicsAdapter *device);

	/* Gets the lowest and highest values in a specified range, excluding the specified default lowest and default highest. */
	template <typename _Ty>
	void _CrtGetValueRange(_In_ _Ty *buffer, _In_ size_t size, _Out_ _Ty *lowest, _Out_ _Ty *highest, _In_opt_ _Ty defaultLowest = minv<_Ty>(), _In_opt_ _Ty defaultHighest = maxv<_Ty>())
	{
		/* Set values to their defaults. */
		*lowest = maxv<_Ty>();
		*highest = minv<_Ty>();

		/* Get range. */
		for (size_t i = 0; i < size; i++)
		{
			const _Ty cur = buffer[i];
			if (cur != defaultLowest && cur < *lowest) *lowest = cur;
			if (cur != defaultHighest && cur > *highest) *highest = cur;
		}

		/* If the lowest or highest was not set; set them to their defaults. */
		if (*lowest == maxv<_Ty>()) *lowest = defaultLowest;
		if (*highest == minv<_Ty>()) *highest = defaultHighest;
	}

	/* 
	Converts the specified raw data into a greyscale image (must be freed using freea_s!).
	The output of this function is RGBA data that has the same element size as the input data.
	The invert option can be used if the lower values should specify bright areas instead of the default darker.
	The normalized option can be used to specifiy that the input values are mapped between zero and one.
	*/
	template<typename _Ty>
	_Check_return_ byte* _CrtToGreyscale(_In_ _Ty *data, _In_ size_t size, _In_opt_ bool invert = false, _In_opt_ bool normalized = false)
	{
		/* Get the range present in the buffer, ignoring zero and one if the buffer is normalized to give a clearer greyscale. */
		_Ty lowest, highest;
		if (normalized) _CrtGetValueRange(data, size, &lowest, &highest, static_cast<_Ty>(0), static_cast<_Ty>(1));
		else _CrtGetValueRange(data, size, &lowest, &highest);

		/* Get the paramters used in the map function later and log the greyscale process start. */
		const float a = invert ? 255.0f : 0.0f;
		const float b = invert ? 0.0f : 255.0f;
		const float c = static_cast<float>(lowest);
		const float d = static_cast<float>(highest);
		LOG("Converting to greyscale, input range: [%f-%f], output range: [0-255].", c, d);

		/* Convert the raw data to greyscale (RGBA format). */
		byte *result = malloca_s(byte, size * 4);
		for (size_t i = 0, j = 0; i < size; i++)
		{
			/* Map raw value to byte range. */
			byte value = static_cast<byte>(map(a, b, static_cast<float>(data[i]), c, d));

			/* Convert mapped value to pre-multiplied opaque color. */
			result[j++] = value;
			result[j++] = value;
			result[j++] = value;
			result[j++] = 255;
		}

		return result;
	}
}