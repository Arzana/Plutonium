#pragma once
#include "Core/Math/Spline.h"
#include "Content/AssetFetcher.h"
#include "Core/Math/Shapes/OBB.h"
#include "Core/Math/Shapes/Line.h"
#include "Core/Math/Shapes/Sphere.h"
#include "Graphics/Cameras/Camera.h"
#include "Graphics/Platform/GameWindow.h"
#include "Graphics/Textures/DepthBuffer.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"

namespace Pu
{
	class DynamicBuffer;
	class QueryChain;

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
		/* Adds a cubic bezier curve to the debug renderer queue, with a specified amount of segments. */
		void AddBezier(_In_ Vector3 start, _In_ Vector3 control, _In_ Vector3 end, _In_ Color color, _In_ uint32 segments);
		/* Adds a quadratic bezier curve to the debug renderer queue, with a specified amount of segments. */
		void AddBezier(_In_ Vector3 start, _In_ Vector3 control1, _In_ Vector3 control2, _In_ Vector3 end, _In_ Color color, _In_ uint32 segments);
		/* Adds a spline to the debug renderer queue, with a specified amount of segments. */
		void AddSpline(_In_ const Spline &spline, _In_ Color color, _In_ uint32 segments);
		/* Adds an arrow to the debug renderer queue. */
		void AddArrow(_In_ Vector3 start, _In_ Vector3 direction, _In_ Color color, _In_opt_ float length = 1.0f);
		/* Adds a matrix transform to the debug renderer queue. */
		void AddTransform(_In_ const Matrix &transform, _In_opt_ float scale = 1.0f, _In_opt_ Vector3 offset = Vector3{});
		/* Adds an axis aligned box to the debug renderer queue. */
		void AddBox(_In_ const AABB &box, _In_ Color color);
		/* Adds a box to the debug renderer queue. */
		void AddBox(_In_ const AABB &box, _In_ const Matrix &transform, _In_ Color color);
		/* Adds a box to the debug renderer queue. */
		void AddBox(_In_ const OBB &box, _In_ Color color);
		/* Adds a sphere to the debug renderer queue. */
		void AddSphere(_In_ Sphere sphere, _In_ Color color);
		/* Adds a sphere to the debug renderer queue. */
		void AddSphere(_In_ Vector3 center, _In_ float radius, _In_ Color color);
		/* Adds an ellipsoid to the debug renderer queue. */
		void AddEllipsoid(_In_ Vector3 center, _In_ float xRadius, _In_ float yRadius, _In_ float zRadius, _In_ Color color);
		/* Adds a capsule to the debug renderer queue. */
		void AddCapsule(_In_ Vector3 center, _In_ float height, _In_ float radius, _In_ Color color);
		/* Adds a frustum to the debug renderer queue. */
		void AddFrustum(_In_ const Frustum &frustum, _In_ Color color);
		/* Renders all shapes stored in the debug renderer. */
		void Render(_In_ CommandBuffer &cmdBuffer, _In_ const Camera &camera, _In_opt_ bool clearBuffer = true);
		/* Resets the debug renderer with a new depth buffer if the old depth buffer changed. */
		void Reset(_In_ const DepthBuffer &depthBuffer);

	private:
		struct VertexLayout
		{
			Matrix mdl;
			Color clr;
		};

		AssetFetcher &loader;
		GameWindow &wnd;
		MeshCollection *meshes;

		Renderpass *renderpass;
		GraphicsPipeline *pipeline;
		QueryChain *query;

		DynamicBuffer *buffer;
		const DepthBuffer *depthBuffer;
		uint32 cntLine, cntArrow, cntBox, cntEllipsoid, cntHemisphere;

#ifdef _DEBUG
		mutable uint32 culled;
#endif

		float lineWidth;
		uint32 invalidated;
		bool dynamicLineWidth, thrown;

		bool CheckCapacity(uint32 list, uint32 addition) const;
		void AddLineInternal(Vector3 start, Vector3 end, Color color);
		void InitializeRenderpass(Renderpass&);
		void InitializePipeline(Renderpass&);
		void CreatePipeline(void);
		void CreateFrameBuffers(void);
		void SwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs &args);
	};
}