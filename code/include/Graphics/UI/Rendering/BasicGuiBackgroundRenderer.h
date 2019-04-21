#pragma once
#include "Graphics/Models/Renderer.h"
#include "GuiBackgroundUniformBlock.h"

namespace Pu
{
	/* Defines a renderer that can be used to render basic UI backgrounds. */
	class BasicGuiBackgroundRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a basic UI background renderer. */
		BasicGuiBackgroundRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_ size_t maxSets);
		BasicGuiBackgroundRenderer(_In_ const BasicGuiBackgroundRenderer&) = delete;
		BasicGuiBackgroundRenderer(_In_ BasicGuiBackgroundRenderer&&) = delete;

		_Check_return_ BasicGuiBackgroundRenderer& operator =(_In_ const BasicGuiBackgroundRenderer&) = delete;
		_Check_return_ BasicGuiBackgroundRenderer& operator =(_In_ BasicGuiBackgroundRenderer&&) = delete;

		/* Creates a uniform block for a specific GUI item (requires delete). */
		_Check_return_ GuiBackgroundUniformBlock* CreateGUI(void) const;
		/* Renders a specific UI background mesh. */
		void Render(_In_ const BufferView &mesh, _In_ const GuiBackgroundUniformBlock &uniforms);

	protected:
		/* Called after the graphics pipeline has been initialized. */
		virtual void OnPipelinePostInitialize(_In_ GraphicsPipeline &pipeline);
		/* Called after the renderpass has completed linking the underlying shaders. */
		virtual void OnRenderpassLinkComplete(_In_ Renderpass &renderpass);
		/* Called when the framebuffers need to be recreated (after window resize event). */
		virtual void RecreateFramebuffers(_In_ GameWindow &window, _In_ const Renderpass &renderpass);
	};
}