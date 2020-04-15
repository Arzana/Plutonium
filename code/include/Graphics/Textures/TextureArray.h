#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines the base object for textures with multiple array layers. */
	class TextureArray
		: public Texture
	{
	public:
		TextureArray(_In_ const TextureArray&) = delete;
		/* Move constructor */
		TextureArray(_In_ TextureArray &&value) = default;

		_Check_return_ TextureArray& operator =(_In_ const TextureArray&) = delete;
		/* Move assignment. */
		_Check_return_ TextureArray& operator =(_In_ TextureArray &&other) = default;

	protected:
		/* Initializes a new instance of an array texture. */
		TextureArray(_In_ Pu::Image &image, _In_ Pu::Sampler &sampler, _In_ ImageViewType type)
			: Texture(&sampler, image, type)
		{}
	};
}