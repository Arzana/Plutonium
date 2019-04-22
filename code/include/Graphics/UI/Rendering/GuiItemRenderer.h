#pragma once
#include <deque>
#include "Content/AssetFetcher.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	class TextRenderer;
	class BasicGuiBackgroundRenderer;
	class GuiItem;
	class Label;

	/* Defines collection of renderers for UI items. */
	class GuiItemRenderer
	{
	public:
		/* Initializes a new instance of a UI item renderer. */
		GuiItemRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_ size_t maxSets);
		GuiItemRenderer(_In_ const GuiItemRenderer&) = delete;
		GuiItemRenderer(_In_ GuiItemRenderer&&) = delete;
		/* Releases the resources allocated by the UI item renderer. */
		~GuiItemRenderer(void);

		_Check_return_ GuiItemRenderer& operator =(_In_ const GuiItemRenderer &other) = delete;
		_Check_return_ GuiItemRenderer& operator =(_In_ GuiItemRenderer &&other) = delete;

		/* Gets whether all components of the UI renderer are ready to render. */
		_Check_return_ bool CanBegin(void) const;
		/* Enqueues the UI item for a basic background render. */
		void EnqueueBackground(_In_ const GuiItem &item, _In_ bool late);
		/* Enqueues the label for a text render. */
		void EnqueueText(_In_ const Label &item);
		/* Renders all enqueues items. */
		void Render(_In_ CommandBuffer &cmdBuffer);

		/* Gets the renderer used to render the background. */
		_Check_return_ inline BasicGuiBackgroundRenderer& GetBackgroundRenderer(void)
		{
			return *backgroundRenderer;
		}

		/* Gets the renderer used to render the text. */
		_Check_return_ inline TextRenderer& GetTextRenderer(void)
		{
			return *textRenderer;
		}

	private:
		BasicGuiBackgroundRenderer *backgroundRenderer;
		TextRenderer *textRenderer;

		std::deque<const GuiItem*> basicEarlyDrawQueue;
		std::deque<const GuiItem*> basicLateDrawQueue;
		std::deque<const Label*> textDrawQueue;

		void UpdateBuffersAndDescriptors(CommandBuffer &cmdBuffer);
		void RenderBasics(CommandBuffer &cmdBuffer, std::deque<const GuiItem*> &queue);
		void RenderText(CommandBuffer &cmdBuffer);
	};
}