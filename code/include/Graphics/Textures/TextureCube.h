#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines a texture made up of 6 2D textures. */
	class TextureCube 
		: public Texture
	{
	public:
		/* Initializes a new instance of a cube texture. */
		TextureCube(_In_ Pu::Image &image, _In_ Pu::Sampler &sampler)
			: Texture(&sampler, image, ImageViewType::ImageCube)
		{}

		TextureCube(_In_ const TextureCube&) = delete;
		/* Move constructor. */
		TextureCube(_In_ TextureCube &&value) = default;

		_Check_return_ TextureCube& operator =(_In_ const TextureCube&) = delete;
		/* Move assignment. */
		_Check_return_ TextureCube& operator =(_In_ TextureCube &&other) = default;
	};
}