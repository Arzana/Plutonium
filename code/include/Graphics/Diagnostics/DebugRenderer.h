#pragma once
#include "Core/Math/Line.h"
#include "Core/Math/AABB.h"
#include "Content/AssetFetcher.h"
#include "Graphics/Platform/GameWindow.h"
#include "Graphics/Textures/DepthBuffer.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"

namespace Pu
{
	class DynamicBuffer;

	/* Defines an obect used to render debug shapes. */
	class DebugRenderer
	{
	public:
		/* Initializes a new instance of a debug renderer. */
		DebugRenderer(_In_ GameWindow &window, _In_ AssetFetcher &loader, _In_opt_ const DepthBuffer *depthBuffer, _In_ float lineWidth);
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

	private: 
		AssetFetcher &loader;
		GameWindow &wnd;

		Renderpass *renderpass;
		GraphicsPipeline *pipeline;

		DynamicBuffer *buffer;
		BufferView *bufferView;
		const DepthBuffer *depthBuffer;
		ColoredVertex3D *queue;
		uint32 size;

		float lineWidth;
		bool dynamicLineWidth;

		void AddVertex(Vector3 p, Color c);
		void InitializeRenderpass(Renderpass&);
		void InitializePipeline(Renderpass&);
		void SwapchainRecreated(const GameWindow&);
	};
}