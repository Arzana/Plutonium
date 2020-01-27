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
		Texture2D(_In_ Pu::Image &image, _In_ Pu::Sampler &sampler)
			: Texture(&sampler, image, ImageViewType::Image2D)
		{}

		Texture2D(_In_ const Texture2D&) = delete;
		/* Move constructor. */
		Texture2D(_In_ Texture2D &&value) = default;

		_Check_return_ Texture2D& operator =(_In_ const Texture2D&) = delete;
		/* Move assignment. */
		_Check_return_ Texture2D& operator =(_In_ Texture2D &&other) = default;

		/* Gets the size of the image. */
		_Check_return_ inline Extent2D GetSize(void) const 
		{
			return Image->GetExtent().To2D();
		}
	};
}