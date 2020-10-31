#pragma once
#include "TextureStorage.h"

namespace Pu
{
	/* Defines a two dimensional storage texture. */
	class TextureStorage2D
		: public TextureStorage
	{
	public:
		/* Initializes a new instance of a 2D storage texture. */
		TextureStorage2D(_In_ Pu::Image &image)
			: TextureStorage(image, ImageViewType::Image2D)
		{}

		TextureStorage2D(_In_ const TextureStorage2D&) = delete;
		/* Move constructor. */
		TextureStorage2D(_In_ TextureStorage2D &&value) = default;

		_Check_return_ TextureStorage2D& operator =(_In_ const TextureStorage2D&) = delete;
		/* Move assignment. */
		_Check_return_ TextureStorage2D& operator =(_In_ TextureStorage2D &&other) = default;

		/* Gets the size of the image. */
		_Check_return_ inline Extent2D GetSize(void) const
		{
			return Image->GetExtent().To2D();
		}
	};
}