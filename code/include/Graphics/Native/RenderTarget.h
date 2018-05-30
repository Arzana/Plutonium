#pragma once
#include "Graphics\Texture.h"

namespace Plutonium
{
	struct GraphicsAdapter;

	/* Defines a basic render target with a color and depth buffer. */
	struct RenderTarget
	{
	public:
		/* Defines the width of the render target. */
		const int32 Width;
		/* Defines the height of the render target. */
		const int32 Height;

		/* Initializes a new instance of a render target. */
		RenderTarget(_In_ GraphicsAdapter *device, _In_ int32 width, _In_ int32 height);
		RenderTarget(_In_ const RenderTarget &value) = delete;
		RenderTarget(_In_ RenderTarget &&value) = delete;
		/* Releases the resources allocated by the render target. */
		~RenderTarget(void);

		_Check_return_ RenderTarget& operator =(_In_ const RenderTarget &other) = delete;
		_Check_return_ RenderTarget& operator =(_In_ RenderTarget &&other) = delete;

		/* Gets the texture associated with the render target. */
		_Check_return_ inline const Texture* GetTexture(void) const
		{
			return texture;
		}

	private:
		friend struct GraphicsAdapter;

		uint32 ptrFb, ptrDb;
		Texture *texture;

		void GenBuffers(void);
		void Configure(void);
	};
}