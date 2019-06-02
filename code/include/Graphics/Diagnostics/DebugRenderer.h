#pragma once
#include "Core/Math/Line.h"
#include "Core/Math/AABB.h"
#include "Graphics/Models/Renderer.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"

namespace Pu
{
	class DebugRendererUniformBlock;
	class DynamicBuffer;

	/* Defines an obect used to render debug shapes. */
	class DebugRenderer
		: private Renderer
	{
	public:
		/* Initializes a new instance of a debug renderer. */
		DebugRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader);
		DebugRenderer(_In_ const DebugRenderer&) = delete;
		DebugRenderer(_In_ DebugRenderer&&) = delete;
		/* Releases the resources allocated by the debug renderer. */
		~DebugRenderer(void);

		_Check_return_ DebugRenderer& operator =(_In_ const DebugRenderer&) = delete;
		_Check_return_ DebugRenderer& operator =(_In_ DebugRenderer&&) = delete;

		/* Adds a single line to the debug renderer. */
		void AddLine(_In_ const Line &line, _In_ Color color);
		/* Adds a single line to the debug renderer queue. */
		void AddLine(_In_ Vector3 start, _In_ Vector3 end, _In_ Color color);
		/* Adds an axis aligned box to the debug renderer queue. */
		void AddBox(_In_ const AABB &box, _In_ Color color);
		/* Adds a box to the debug renderer queue. */
		void AddBox(_In_ const AABB &box, _In_ const Matrix &transform, _In_ Color color);
		/* Renders all shapes stored in the debug renderer. */
		void Render(_In_ CommandBuffer &cmdBuffer, _In_ const Matrix &projection, _In_ const Matrix &view);

	protected:
		/* Called after the graphics pipeline has been initialized. */
		virtual void OnPipelinePostInitialize(_In_ GraphicsPipeline &pipeline);
		/* Called after the renderpass has completed linking the underlying shaders. */
		virtual void OnRenderpassLinkComplete(_In_ Renderpass &renderpass);
		/* Called when the framebuffers need to be recreated (after window resize event). */
		virtual void RecreateFramebuffers(_In_ GameWindow &window, _In_ const Renderpass &renderpass);

	private: 
		DebugRendererUniformBlock *uniforms;
		DynamicBuffer *buffer;
		BufferView *bufferView;
		ColoredVertex3D *queue;
		uint32 size;

		void AddVertex(Vector3 p, Color c);
	};
}