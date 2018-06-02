#include "Graphics\Rendering\StitchedTexture2D.h"
#include "Graphics\Rendering\SpriteRenderer.h"

Plutonium::StitchedTexture2D::StitchedTexture2D(int32 width, int32 height, GraphicsAdapter * device)
	: std::map<int32, Rectangle>(), device(device)
{
	/* Create render target and texture renderer. */
	target = new RenderTarget(device, RenderTargetType::Color, width, height);
	renderer = new SpriteRenderer(device);
}

Plutonium::StitchedTexture2D::~StitchedTexture2D(void)
{
	delete_s(target);
	delete_s(renderer);
}

void Plutonium::StitchedTexture2D::Start(void)
{
	/* Clear the dictionary of its previous values. */
	clear();

	/* Bind the render target and clear it from its previous use. */
	device->SetRenderTarget(target);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	device->Clear(ClearTarget::Color);
	renderer->Begin();
}

void Plutonium::StitchedTexture2D::RenderAt(int32 id, const Texture * texture, Vector2 position)
{
	/* Add the texture to the dictionary and render it at the specified location. */
	insert(std::pair<int32, Rectangle>(id, Rectangle(position, texture->GetSize())));
	renderer->Render(texture, position);
}

void Plutonium::StitchedTexture2D::RenderAt(int32 parentId, const StitchedTexture2D * texture, Vector2 position)
{
	/* Add the texture getters into our current texture. */
	for (std::map<int32, Rectangle>::const_iterator it = texture->begin(); it != texture->end(); it++)
	{
		insert(std::pair<int32, Rectangle>(parentId | it->first, Rectangle(it->second.Position + position, it->second.Size)));
	}

	/* Render texture at the specified location. */
	renderer->Render(texture->GetTexture(), position);
}

void Plutonium::StitchedTexture2D::End(void)
{
	/* Log texture creation. */
	LOG("Finished stitching texture %dx%d, %zu underlying textures.", target->Width, target->Height, size());

	/* Reset the render target to the window and end the renderer. */
	device->SetRenderTarget(nullptr);
	renderer->End();
}