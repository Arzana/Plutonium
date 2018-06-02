#pragma once
#include <map>
#include "Graphics\Native\RenderTarget.h"

namespace Plutonium
{
	struct SpriteRenderer;

	/* Defines a container for multiple textures stitched into one texture. */
	struct StitchedTexture2D
		: private std::map<int32, Rectangle>
	{
	public:
		/* Initializes a new instance of a 2D stitched texture. */
		StitchedTexture2D(_In_ int32 width, _In_ int32 height, _In_ GraphicsAdapter *device);
		StitchedTexture2D(_In_ const StitchedTexture2D &value) = delete;
		StitchedTexture2D(_In_ StitchedTexture2D &&value) = delete;
		/* Releases the resources allocated by the stitched texture. */
		~StitchedTexture2D(void);

		_Check_return_ StitchedTexture2D& operator =(_In_ const StitchedTexture2D &other) = delete;
		_Check_return_ StitchedTexture2D& operator =(_In_ StitchedTexture2D &&other) = delete;

		/* Starts the rendering process. */
		void Start(void);
		/* Adds a single texture to the stitched map with a specified id and a specified location. */
		void RenderAt(_In_ int32 id, _In_ const Texture *texture, _In_ Vector2 position);
		/* Adds multiple textures to the stitched map with a specified parent id and a specified location. */
		void RenderAt(_In_ int32 parentId, _In_ const StitchedTexture2D *texture, _In_ Vector2 position);
		/* Ends the renering process. */
		void End(void);

		/* Gets the result of the texture stitching process. */
		_Check_return_ inline const Texture* GetTexture(void) const
		{
			return target->GetTexture();
		}

	private:
		GraphicsAdapter *device;
		RenderTarget *target;
		SpriteRenderer *renderer;
	};
}