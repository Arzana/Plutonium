#pragma once
#include "Graphics\Renderer.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Texture.h"

namespace Plutonium
{
	struct Buffer;

	/* Defines a very basic sprite renderer. */
	struct SpriteRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a basic sprite renderer. */
		SpriteRenderer(_In_ GraphicsAdapter *device, _In_ const char *vrtxShdr, _In_ const char * fragShdr);
		SpriteRenderer(_In_ const SpriteRenderer &value) = delete;
		SpriteRenderer(_In_ SpriteRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~SpriteRenderer(void);

		_Check_return_ SpriteRenderer& operator =(_In_ const SpriteRenderer &other) = delete;
		_Check_return_ SpriteRenderer& operator =(_In_ SpriteRenderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(void);
		/* Renders the specified sprite. */
		void Render(_In_ const Texture *sprite, _In_ Vector2 position, _In_opt_ Color color = Color::White, _In_opt_ Vector2 scale = Vector2::One, _In_opt_ float rotation = 0.0f);

	protected:
		GraphicsAdapter *device;

	private:
		Uniform *matVp, *matMdl, *texture, *color;
		Attribute *posUv;
		Buffer *mesh;
		Matrix proj;

		void WindowResizeEventHandler(WindowHandler sender, EventArgs args);
		void UpdateVBO(Vector2 size);
	};
}