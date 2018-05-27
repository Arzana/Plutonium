#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\Buffer.h"

namespace Plutonium
{
	/* Defines a base object for renderers. */
	struct Renderer
	{
	public:
		Renderer(_In_ const Renderer &value) = delete;
		Renderer(_In_ Renderer &&value) = delete;
		/* Releases the shader allocated by the renderer. */
		~Renderer(void);

		_Check_return_ Renderer& operator =(_In_ const Renderer &other) = delete;
		_Check_return_ Renderer& operator =(_In_ Renderer &&other) = delete;

		/* Starts the rendering process. */
		virtual void Begin(void);
		/* Ends the rendering process. */
		virtual void End(void);

	protected:
		/* Gets the shader associated with the renderer. */
		_Check_return_ inline const Shader* GetShader(void) const
		{
			return shdr;
		}

		/* Signals OpenGL to draw the specified buffer as triangles. */
		void DrawTris(_In_ const Buffer *buffer, _In_opt_ int32 start = 0);

		/* Initializes a new instance of a renderer (shader should be made with new and is deleted by the renderer!). */
		Renderer(_In_ Shader *shader);

	private:
		bool beginCalled;
		Shader *shdr;
	};
}