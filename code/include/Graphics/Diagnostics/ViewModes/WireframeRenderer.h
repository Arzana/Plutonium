#pragma once
#include "Graphics\Renderer.h"
#include "Graphics\Mesh.h"

namespace Plutonium
{
	/* Defines a renderer that can view wireframes. */
	struct WireframeRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a wireframe renderer. */
		WireframeRenderer(void);
		WireframeRenderer(_In_ const WireframeRenderer &value) = delete;
		WireframeRenderer(_In_ WireframeRenderer &&value) = delete;

		_Check_return_ WireframeRenderer& operator =(_In_ const WireframeRenderer &other) = delete;
		_Check_return_ WireframeRenderer& operator =(_In_ WireframeRenderer &&other) = delete;

		/* Starts the renderer. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj);
		/* Renders a single shape's wireframe. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh, _In_opt_ Color color = Color::White);
		/* Ends the renderer. */
		void End(void);

	private:
		Uniform *matMdl, *matView, *matProj, *clr;
		Attribute *pos;
	};
}