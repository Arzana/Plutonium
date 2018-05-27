#pragma once
#include "Graphics\Renderer.h"
#include "Graphics\Mesh.h"
#include "Graphics\Texture.h"

namespace Plutonium
{
	/* Defines a renderer that renders an unlit scene. */
	struct UnlitRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of an unlit renderer. */
		UnlitRenderer(void);
		UnlitRenderer(_In_ const UnlitRenderer &value) = delete;
		UnlitRenderer(_In_ UnlitRenderer &&value) = delete;

		_Check_return_ UnlitRenderer& operator =(_In_ const UnlitRenderer &other) = delete;
		_Check_return_ UnlitRenderer& operator =(_In_ UnlitRenderer &&other) = delete;

		/* Starts the rendering process. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj);
		/* Renders a specified mesh with a specified base texture. */
		void Render(_In_ const Matrix &world, _In_ const Mesh *mesh, _In_ const Texture *base, _In_ const Texture *alpha);

	private:
		Uniform *matmdl, *matView, *matProj, *mapBase, *mapAlpha;
		Attribute *pos, *uv;
	};
}