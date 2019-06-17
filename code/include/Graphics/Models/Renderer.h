#pragma once
#include "Content/AssetFetcher.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	/* Defines a base class for renderers to minimize boilerplate code. */
	class Renderer
	{
	public:
		/* Initializes a new instance of a base renderer. */
		Renderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_ size_t maxSets, _In_ std::initializer_list<wstring> shaders);
		Renderer(_In_ const Renderer&) = delete;
		Renderer(_In_ Renderer&&) = delete;
		/* Releases the resources allocated by the renderer. */
		virtual ~Renderer(void);

		_Check_return_ Renderer& operator =(_In_ const Renderer&) = delete;
		_Check_return_ Renderer& operator =(_In_ Renderer&&) = delete;

		/* Checkes whether the graphics pipeline can be started. */
		_Check_return_ bool CanBegin(void);
		/* Starts the graphics pipeline. */
		virtual void Begin(_In_ CommandBuffer &cmdBuffer);
		/* Ends the graphics pipeline. */
		virtual void End(void);

	protected:
		/* The command buffer used to start the renderer. */
		CommandBuffer *CmdBuffer;

		/* Called after the graphics pipeline has been initialized. */
		virtual void OnPipelinePostInitialize(_In_ GraphicsPipeline &pipeline) = 0;
		/* Called after the renderpass has completed linking the underlying shaders. */
		virtual void OnRenderpassLinkComplete(_In_ Renderpass &renderpass) = 0;
		/* Called when the framebuffers need to be recreated (after window resize event). */
		virtual void RecreateFramebuffers(_In_ GameWindow &window, _In_ const Renderpass &renderpass) = 0;

		/* Gets the window used to create the renderer. */
		_Check_return_ inline GameWindow& GetWindow(void)
		{
			return wnd;
		}

		/* Gets the window used to create the renderer. */
		_Check_return_ inline const GameWindow& GetWindow(void) const
		{
			return wnd;
		}

		/* Gets the graphics pipeline of the renderer. */
		_Check_return_ inline GraphicsPipeline& GetPipeline(void)
		{
			return *pipeline;
		}

		/* Gets the graphics pipeline of the renderer. */
		_Check_return_ inline const GraphicsPipeline& GetPipeline(void) const
		{
			return *pipeline;
		}

	private:
		GameWindow &wnd;
		AssetFetcher &loader;
		GraphicsPipeline *pipeline;

		void OnSwapchainChanged(const GameWindow&);
	};
}