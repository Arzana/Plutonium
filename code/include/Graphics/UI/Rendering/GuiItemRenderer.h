#pragma once
#include <deque>
#include "Content/AssetFetcher.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	class BasicGuiBackgroundRenderer;
	class GuiItem;

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

		/* Enqueues the UI item for a basic background render. */
		void EnqueueBackground(_In_ const GuiItem &item, _In_ bool late);
		/* Renders all enqueues items. */
		void Render(_In_ CommandBuffer &cmdBuffer);

		/* Gets the renderer used to render the background. */
		_Check_return_ inline BasicGuiBackgroundRenderer& GetBackgroundRenderer(void) const
		{
			return *backgroundRenderer;
		}

	private:
		BasicGuiBackgroundRenderer *backgroundRenderer;

		std::deque<const GuiItem*> basicEarlyDrawQueue;
		std::deque<const GuiItem*> basicLateDrawQueue;

		void UpdateBuffersAndDescriptors(CommandBuffer &cmdBuffer);
		void RenderBasics(CommandBuffer &cmdBuffer, std::deque<const GuiItem*> &queue);
	};
}