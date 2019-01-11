#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines a two dimensional texture. */
	class Texture2D
		: public Texture
	{
	public:
		/* Initializes a new instance of a 2D texture. */
		Texture2D(_In_ LogicalDevice &device, _In_ Sampler &sampler, _In_ Format format, _In_ Extent2D extent, _In_ ImageUsageFlag usage);
		/* Initializes a new instance of a 2D texture with the specified amount of mipmap levels, array layers and samples. */
		Texture2D(_In_ LogicalDevice &device, _In_ Sampler &sampler, _In_ Format format, _In_ Extent2D extent, _In_ ImageUsageFlag usage, _In_ uint32 mipLevels, _In_ uint32 arrayLayers, _In_ SampleCountFlag samples);
		Texture2D(_In_ const Texture2D&) = delete;
		/* Move constructor. */
		Texture2D(_In_ Texture2D &&value);

		_Check_return_ Texture2D& operator =(_In_ const Texture&) = delete;
		/* Move assignment. */
		_Check_return_ Texture2D& operator =(_In_ Texture &&other);

		/* Gets the size of the image. */
		_Check_return_ inline Extent2D GetSize(void) const 
		{
			return GetExtent().To2D();
		}
	};
}