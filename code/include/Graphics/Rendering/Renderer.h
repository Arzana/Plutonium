#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Window.h"
#include "Graphics\Models\Model.h"

namespace Plutonium
{
	/* Defines a very basic model renderer. */
	struct Renderer
	{
	public:
		/* Initializes a new instance of a basic model renderer. */
		Renderer(_In_ const char *vrtxShdr, _In_ const char * fragShdr);
		Renderer(_In_ const Renderer &value) = delete;
		Renderer(_In_ Renderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~Renderer(void);

		_Check_return_ Renderer& operator =(_In_ const Renderer &other) = delete;
		_Check_return_ Renderer& operator =(_In_ Renderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(_In_ const Matrix &view, const Matrix &proj, _In_ Vector3 lightDir);
		/* Renders the specified model. */
		void Render(_In_ const Model *model);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj, *texture, *lightDir;
		Attribute *pos, *norm, *uv;
		bool beginCalled;
	};
}