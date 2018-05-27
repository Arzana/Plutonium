#pragma once
#include "Graphics\Renderer.h"
#include "Graphics\Mesh.h"
#include "Graphics\Texture.h"

namespace Plutonium
{
	/* Defines a renderer that can render normals. */
	struct NormalRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a normal renderer. */
		NormalRenderer(void);
		NormalRenderer(_In_ const NormalRenderer &value) = delete;
		NormalRenderer(_In_ NormalRenderer &&value) = delete;

		_Check_return_ NormalRenderer& operator =(_In_ const NormalRenderer &other) = delete;
		_Check_return_ NormalRenderer& operator =(_In_ NormalRenderer &&other) = delete;

		/* Starts the renderer. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj);
		/* Renders a single shape's normals. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh, _In_ const Texture *bumpMap);

	private:
		Uniform * matMdl, *matView, *matProj, *mapBmp;
		Attribute *pos, *norm, *tan, *uv;
	};
}