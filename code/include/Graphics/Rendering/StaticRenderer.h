#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Models\StaticModel.h"

namespace Plutonium
{
	/* Defines a very basic model renderer. */
	struct StaticRenderer
	{
	public:
		/* Initializes a new instance of a basic model renderer. */
		StaticRenderer(_In_ const char *vrtxShdr, _In_ const char * fragShdr);
		StaticRenderer(_In_ const StaticRenderer &value) = delete;
		StaticRenderer(_In_ StaticRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~StaticRenderer(void);

		_Check_return_ StaticRenderer& operator =(_In_ const StaticRenderer &other) = delete;
		_Check_return_ StaticRenderer& operator =(_In_ StaticRenderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Vector3 lightDir);
		/* Renders the specified model. */
		void Render(_In_ const StaticModel *model);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj, *texture, *lightDir;
		Attribute *pos, *norm, *uv;
		bool beginCalled;
	};
}