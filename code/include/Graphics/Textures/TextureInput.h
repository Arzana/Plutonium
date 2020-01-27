#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines the base object for textures that will be used as input attachments. */
	class TextureInput
		: public Texture
	{
	public:
		TextureInput(_In_ const TextureInput&) = delete;
		/* Move constructor. */
		TextureInput(_In_ TextureInput &&value) = default;
		/* Releases the resources stored in the input texture. */
		virtual ~TextureInput(void)
		{
			Destroy();
		}

		_Check_return_ TextureInput& operator =(_In_ const TextureInput&) = delete;
		/* Move assignment. */
		_Check_return_ TextureInput& operator =(_In_ TextureInput &&other);

	protected:
		/* Initializes a new instance of a input texture (takes ownership of the image!). */
		TextureInput(_In_ Pu::Image &image, _In_ ImageViewType type);

	private:
		void Destroy(void);
	};
}