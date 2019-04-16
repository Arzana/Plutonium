#pragma once
#include "TextBuffer.h"
#include "TextUniformBlock.h"
#include "Content/AssetFetcher.h"
#include "ConstTextUniformBlock.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	/* Defines an object used to render raw strings. */
	class TextRenderer
	{
	public:
		/* Initializes a new insance of a text renderer to a specific window. */
		TextRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_ size_t maxSets, _In_ std::initializer_list<wstring> shaders);
		TextRenderer(_In_ const TextRenderer&) = delete;
		TextRenderer(_In_ TextRenderer&&) = delete;
		/* Releases the resource allocated by the textrenderer. */
		~TextRenderer(void);

		_Check_return_ TextRenderer& operator =(_In_ const TextRenderer&) = delete;
		_Check_return_ TextRenderer& operator =(_In_ TextRenderer&&) = delete;

		/* Checks whether the graphics pipeline can be started. */
		_Check_return_ bool CanBegin(void) const;
		/* Creates a uniform block for string specific information for this text renderer. */
		_Check_return_ TextUniformBlock CreateText(void) const;
		/* Creates a uniform block for the constant information for this text renderer. */
		_Check_return_ ConstTextUniformBlock CreatFont(void) const;
		/* Starts the graphics pipeline. */
		void Begin(_In_ CommandBuffer &cmdBuffer);
		/* Sets the font to use. */
		void SetFont(_In_ const ConstTextUniformBlock &info);
		/* Renders a specific text mesh. */
		void Render(_In_ const TextBuffer &text, _In_ const TextUniformBlock &uniforms);
		/* Ends the graphics pipeline. */
		void End(void);

	private:
		GameWindow &wnd;
		AssetFetcher &loader;
		GraphicsPipeline *pipeline;
		CommandBuffer *curCmdBuffer;

		void OnPipelinePostInitialize(GraphicsPipeline&);
		void OnRenderpassLinkComplete(Renderpass &renderpass);
		void OnWindowSizeChanged(const NativeWindow&, ValueChangedEventArgs<Vector2>);
	};
}