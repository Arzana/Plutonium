#pragma once
#include "TextBuffer.h"
#include "TextUniformBlock.h"
#include "Graphics/Models/Renderer.h"

namespace Pu
{
	/* Defines an object used to render raw strings. */
	class TextRenderer
		: public Renderer
	{
	public:
		/* Initializes a new insance of a text renderer to a specific window. */
		TextRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_ size_t maxSets);
		TextRenderer(_In_ const TextRenderer&) = delete;
		TextRenderer(_In_ TextRenderer&&) = delete;

		_Check_return_ TextRenderer& operator =(_In_ const TextRenderer&) = delete;
		_Check_return_ TextRenderer& operator =(_In_ TextRenderer&&) = delete;

		/* Creates a uniform block for string specific information for this text renderer. */
		_Check_return_ TextUniformBlock CreateText(void) const;
		/* Creates a uniform block for the constant information for this text renderer. */
		_Check_return_ DescriptorSet CreatFont(_In_ const Texture2D &atlas) const;
		/* Starts the graphics pipeline. */
		virtual void Begin(_In_ CommandBuffer &cmdBuffer);
		/* Sets the font to use. */
		void SetFont(_In_ const DescriptorSet &info);
		/* Renders a specific text mesh. */
		void Render(_In_ const TextBuffer &text, _In_ const TextUniformBlock &uniforms);
		/* Ends the graphics pipeline. */
		virtual void End(void);

	protected:
		/* Called after the graphics pipeline has been initialized. */
		virtual void OnPipelinePostInitialize(_In_ GraphicsPipeline &pipeline);
		/* Called after the renderpass has completed linking the underlying shaders. */
		virtual void OnRenderpassLinkComplete(_In_ Renderpass &renderpass);
		/* Called when the framebuffers need to be recreated (after window resize event). */
		virtual void RecreateFramebuffers(_In_ GameWindow &window, _In_ const Renderpass &renderpass);
	};
}