#pragma once
#include "Graphics\Texture.h"

namespace Plutonium
{
	struct GraphicsAdapter;

	/* Defines the layout and use of a render target. */
	enum class RenderTargetType
	{
		/* The RenderTarget stores color information and doesn't have a depth buffer. */
		Color,
		/* The RenderTarget stores color information and performs depth testing. */
		ColorDepth,
		/* The RenderTarget stores depth information and performs depth testing. */
		Depth
	};

	/* Defines a basic render target with a specified type. */
	struct RenderTarget 
	{
	public:
		/* Defines the width of the render target's viewport. */
		const int32 Width;
		/* Defines the height of the redner target's viewport. */
		const int32 Height;

		/* Initializes a new instance of a render target. */
		RenderTarget(_In_ GraphicsAdapter *device, _In_ RenderTargetType type, _In_ int32 width, _In_ int32 height);
		RenderTarget(_In_ const RenderTarget &value) = delete;
		RenderTarget(_In_ RenderTarget &&value) = delete;
		/* Releases the resources allocated by the render target. */
		~RenderTarget(void);

		_Check_return_ RenderTarget& operator =(_In_ const RenderTarget &other) = delete;
		_Check_return_ RenderTarget& operator =(_In_ RenderTarget &&other) = delete;

		/* Gets the underlying texture buffer. */
		_Check_return_ inline TextureHandler GetTexture(void) const
		{
			return target;
		}

	private:
		friend struct GraphicsAdapter;

		uint32 ptrFb, ptrDb;
		Texture *target;

		void GenBuffers(bool addDepthComponent);
		void Configure(bool isDepth);
	};
}