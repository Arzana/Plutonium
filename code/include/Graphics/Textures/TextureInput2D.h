#pragma once
#include "TextureInput.h"

namespace Pu
{
	/* Defines a two dimensional input texture. */
	class TextureInput2D
		: public TextureInput
	{
	public:
		/* Initializes a new instance of a 2D input texture. */
		TextureInput2D(_In_ Pu::Image &image)
			: TextureInput(image, ImageViewType::Image2D)
		{}

		TextureInput2D(_In_ const TextureInput2D&) = delete;
		/* Move constructor. */
		TextureInput2D(_In_ TextureInput2D &&value) = default;

		_Check_return_ TextureInput2D& operator =(_In_ const TextureInput2D&) = delete;
		/* Move assignment. */
		_Check_return_ TextureInput2D& operator =(_In_ TextureInput2D &&other) = default;

		/* Gets the size of the image. */
		_Check_return_ inline Extent2D GetSize(void) const
		{
			return Image->GetExtent().To2D();
		}
	};
}