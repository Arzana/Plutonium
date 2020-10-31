#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines the base object for textures that will be used as storage textures. */
	class TextureStorage
		: public Texture
	{
	public:
		TextureStorage(_In_ const TextureStorage&) = delete;
		/* Move constructor. */
		TextureStorage(_In_ TextureStorage &&value) = default;
		/* Releases the resources atored in the storage texture. */
		virtual ~TextureStorage(void)
		{
			Destroy();
		}

		_Check_return_ TextureStorage& operator =(_In_ const TextureStorage&) = delete;
		/* Move assignment. */
		_Check_return_ TextureStorage& operator =(_In_ TextureStorage &&other);

	protected:
		/* Initializes a new instance of a storage texture (takes ownership of the image!). */
		TextureStorage(_In_ Pu::Image &image, _In_ ImageViewType type);

	private:
		void Destroy(void);
	};
}