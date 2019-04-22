#include "Graphics/UI/Rendering/GuiItemRenderer.h"
#include "Graphics/UI/Rendering/BasicGuiBackgroundRenderer.h"
#include "Graphics/Text/TextRenderer.h"
#include "Graphics/UI/Core/GuiItem.h"
#include "Graphics/UI/Items/Label.h"

Pu::GuiItemRenderer::GuiItemRenderer(GameWindow & window, AssetFetcher & loader, size_t maxSets)
{
	backgroundRenderer = new BasicGuiBackgroundRenderer(window, loader, maxSets);
	textRenderer = new TextRenderer(window, loader, maxSets);
}

Pu::GuiItemRenderer::~GuiItemRenderer(void)
{
	delete backgroundRenderer;
	delete textRenderer;
}

bool Pu::GuiItemRenderer::CanBegin(void) const
{
	return backgroundRenderer->CanBegin() && textRenderer->CanBegin();
}

void Pu::GuiItemRenderer::EnqueueBackground(const GuiItem & item, bool late)
{
	if (late) basicLateDrawQueue.push_back(&item);
	else basicEarlyDrawQueue.push_back(&item);
}

void Pu::GuiItemRenderer::EnqueueText(const Label & item)
{
	textDrawQueue.push_back(&item);
}

void Pu::GuiItemRenderer::Render(CommandBuffer & cmdBuffer)
{
	cmdBuffer.AddLabel(u8"UI", Color::Abbey());

	UpdateBuffersAndDescriptors(cmdBuffer);
	// Render split
	RenderBasics(cmdBuffer, basicEarlyDrawQueue);
	RenderText(cmdBuffer);
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

	for (const Label *cur : textDrawQueue)
	{
		cur->textBuffer->Update(cmdBuffer);
		cur->textDescriptor->Update(cmdBuffer);
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

void Pu::GuiItemRenderer::RenderText(CommandBuffer & cmdBuffer)
{
	/* Early out to avoid not needed GPU commands. */
	if (textDrawQueue.empty()) return;

	textRenderer->Begin(cmdBuffer);

	while (!textDrawQueue.empty())
	{
		/* 
		Can be optimized by presorting the labels with their fonts in mind,
		currently it's not worth the efford.
		*/
		const Label *cur = textDrawQueue.front();
		textRenderer->SetFont(*cur->fontDescriptor);
		textRenderer->Render(*cur->textBuffer, *cur->textDescriptor);
		textDrawQueue.pop_front();
	}

	textRenderer->End();
} 