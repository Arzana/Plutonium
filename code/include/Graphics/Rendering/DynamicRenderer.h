#pragma once
#include "Graphics\Renderer.h"
#include "GameLogic\DynamicObject.h"

namespace Plutonium
{
	/* Defines a very basic model renderer. */
	struct DynamicRenderer
		: public Renderer
	{
	public:
		/* Initializes a new instance of a basic model renderer. */
		DynamicRenderer(_In_ const char *vrtxShdr, _In_ const char * fragShdr);
		DynamicRenderer(_In_ const DynamicRenderer &value) = delete;
		DynamicRenderer(_In_ DynamicRenderer &&value) = delete;

		_Check_return_ DynamicRenderer& operator =(_In_ const DynamicRenderer &other) = delete;
		_Check_return_ DynamicRenderer& operator =(_In_ DynamicRenderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Vector3 lightDir);
		/* Renders the specified model. */
		void Render(_In_ const DynamicObject *model);

	private:
		Uniform *matMdl, *matView, *matProj, *texture, *lightDir, *ambient, *time;
		Attribute *pos1, *pos2, *norm1, *norm2, *uv;
	};
}