#include "Graphics/UI/Rendering/GuiItemRenderer.h"
#include "Graphics/UI/Rendering/BasicGuiBackgroundRenderer.h"
#include "Graphics/UI/Core/GuiItem.h"

Pu::GuiItemRenderer::GuiItemRenderer(GameWindow & window, AssetFetcher & loader, size_t maxSets)
{
	backgroundRenderer = new BasicGuiBackgroundRenderer(window, loader, maxSets);
}

Pu::GuiItemRenderer::~GuiItemRenderer(void)
{
	delete backgroundRenderer;
}

void Pu::GuiItemRenderer::EnqueueBackground(const GuiItem & item, bool late)
{
	if (late) basicLateDrawQueue.push_back(&item);
	else basicEarlyDrawQueue.push_back(&item);
}

void Pu::GuiItemRenderer::Render(CommandBuffer & cmdBuffer)
{
	cmdBuffer.AddLabel(u8"UI", Color::Abbey());

	UpdateBuffersAndDescriptors(cmdBuffer);
	// Render split
	RenderBasics(cmdBuffer, basicEarlyDrawQueue);
	// Render text
	// Render bars
	RenderBasics(cmdBuffer, basicLateDrawQueue);

	cmdBuffer.EndLabel();
}

void Pu::GuiItemRenderer::UpdateBuffersAndDescriptors(CommandBuffer & cmdBuffer)
{
	for (const GuiItem *cur : basicEarlyDrawQueue)
	{
		cur->buffer->Update(cmdBuffer);
		cur->backgroundDescriptor->Update(cmdBuffer);
	}

	for (const GuiItem *cur : basicLateDrawQueue)
	{
		cur->buffer->Update(cmdBuffer);
		cur->backgroundDescriptor->Update(cmdBuffer);
	}
}

void Pu::GuiItemRenderer::RenderBasics(CommandBuffer & cmdBuffer, std::deque<const GuiItem*> & queue)
{
	/* Early out to avoid not needed GPU commands. */
	if (queue.empty()) return;

	backgroundRenderer->Begin(cmdBuffer);

	while (!queue.empty())
	{
		const GuiItem *cur = queue.front();
		backgroundRenderer->Render(*cur->view, *cur->backgroundDescriptor);
		queue.pop_front();
	}

	backgroundRenderer->End();
}