#pragma once
#include "TextureArray.h"

namespace Pu
{
	/* Defines a 2D texture with multiple array layers. */
	class Texture2DArray
		: public TextureArray
	{
	public:
		/* Initializes a new instance of a 2D texture array. */
		Texture2DArray(_In_ Pu::Image &image, _In_ Pu::Sampler &sampler)
			: TextureArray(image, sampler, ImageViewType::Image2DArray)
		{}

		Texture2DArray(_In_ const Texture2DArray&) = delete;
		/* Move constructor. */
		Texture2DArray(_In_ Texture2DArray &&value) = default;

		_Check_return_ Texture2DArray& operator =(_In_ const Texture2DArray&) = delete;
		/* Move assignment. */
		_Check_return_ Texture2DArray& operator =(_In_ Texture2DArray &&other) = default;

		/* Gets the size of the image. */
		_Check_return_ inline Extent2D GetSize(void) const
		{
			return Image->GetExtent().To2D();
		}

	protected:
		friend class AssetFetcher;

		/* Pass-through constructor (used for TextureCube). */
		Texture2DArray(_In_ Pu::Image &image, _In_ Pu::Sampler &sampler, _In_ ImageViewType type)
			: TextureArray(image, sampler, type)
		{}
	};
}