#pragma once
#include "Shader.h"
#include "Graphics\Models\DynamicModel.h"

namespace Plutonium
{
	/* Defines a very basic model renderer. */
	struct DynamicRenderer
	{
	public:
		/* Initializes a new instance of a basic model renderer. */
		DynamicRenderer(_In_ const char *vrtxShdr, _In_ const char * fragShdr);
		DynamicRenderer(_In_ const DynamicRenderer &value) = delete;
		DynamicRenderer(_In_ DynamicRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~DynamicRenderer(void);

		_Check_return_ DynamicRenderer& operator =(_In_ const DynamicRenderer &other) = delete;
		_Check_return_ DynamicRenderer& operator =(_In_ DynamicRenderer &&other) = delete;

		/* Starts rendering the specified scene. */
		void Begin(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Vector3 lightDir);
		/* Renders the specified model. */
		void Render(_In_ const DynamicModel *model);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj, *texture, *lightDir, *time;
		Attribute *pos1, *pos2, *norm1, *norm2, *uv;
		bool beginCalled;
	};
}