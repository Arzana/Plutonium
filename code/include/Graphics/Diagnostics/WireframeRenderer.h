#pragma once
#include "Graphics\Rendering\Shader.h"
#include "GameLogic\StaticObject.h"
#include "GameLogic\DynamicObject.h"

namespace Plutonium
{
	/* Defines a very basic mesh/wireframe renderer. */
	struct WireframeRenderer
	{
	public:
		/* Initializes a new instance of a basic mesh renderer. */
		WireframeRenderer(_In_ const char *vrtxShdr);
		WireframeRenderer(_In_ const WireframeRenderer &value) = delete;
		WireframeRenderer(_In_ WireframeRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~WireframeRenderer(void);

		_Check_return_ WireframeRenderer& operator =(_In_ const WireframeRenderer &other) = delete;
		_Check_return_ WireframeRenderer& operator =(_In_ WireframeRenderer &&other) = delete;

		/* Start rendering the specified scene. */
		void Begin(_In_ const Matrix &view, const Matrix &proj);
		/* Renders the specified model as a wireframe. */
		void Render(_In_ const StaticObject *model, _In_opt_ Color color = Color::Red);
		/* Render the specified model as a wireframe. */
		void Render(_In_ const DynamicObject *model, _In_opt_ Color color = Color::Yellow);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj, *clr;
		Attribute *pos;
		bool beginCalled;
	};
}