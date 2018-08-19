#pragma once
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Renderer.h"
#include "Core\Math\Box.h"

namespace Plutonium
{
	/* Defines a basic renderer used to render primitive shapes. */
	struct ShapeRenderer
		: private Renderer
	{
	public:
		/* Initializes a new instance of a debug renderer. */
		ShapeRenderer(_In_ const GraphicsAdapter *device, _In_opt_ size_t bufferSize = 4096);
		ShapeRenderer(_In_ const ShapeRenderer &value) = delete;
		ShapeRenderer(_In_ ShapeRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~ShapeRenderer(void);

		_Check_return_ ShapeRenderer& operator =(_In_ const ShapeRenderer &other) = delete;
		_Check_return_ ShapeRenderer& operator =(_In_ ShapeRenderer &&other) = delete;

		/* Adds a ray to the render queue. */
		void AddRay(_In_ Vector3 start, _In_ Vector3 end, _In_opt_ Color clr = Color::Yellow());
		/* Adds a matrix or coordinate system to the render queue. */
		void AddMatrix(_In_ const Matrix &m, _In_opt_ Color xClr = Color::Green(), _In_opt_ Color yClr = Color::Blue(), _In_opt_ Color zClr = Color::Red());
		/* Adds a circle to the render queue. */
		void AddCircle(_In_ Vector3 center, _In_ float radius, _In_opt_ Color clr = Color::Green(), _In_opt_ int32 divs = 12);
		/* Adds a sphere to the render queue. */
		void AddSphere(_In_ Vector3 center, _In_ float radius, _In_opt_ Color xzClr = Color::Green(), _In_opt_ Color xyClr = Color::Blue(), _In_opt_ Color yzClr = Color::Red(), _In_opt_ int32 divs = 12);
		/* Adds a box to the render queue. */
		void AddAABB(_In_ const Box &box, _In_opt_ Color clr = Color::Yellow());
		/* Adds a box to the render queue. */
		void AddOBB(_In_ const Box &box, _In_ const Matrix &orien, _In_opt_ Color clr = Color::Yellow());
		/* Renders the queued primitives. */
		void Render(_In_ const Matrix &view, _In_ const Matrix &projection);

	private:
		struct VertexFormat
		{
			Vector3 Pos;
			Vector4 Clr;
		} *queue;

		Buffer *vbo;
		size_t lineCnt, maxLineCnt;

		const GraphicsAdapter *device;
		Uniform *matView, *matProj;
		Attribute *pos, *clr;

		void AllocVbo(void);
		void InitShader(void);
	};
}