#pragma once
#include "Graphics\Renderer.h"
#include "Graphics\GraphicsAdapter.h"

namespace Plutonium
{
	/* Defines a way of rendering cubemaps. */
	class SkyboxRenderer
		: private Renderer
	{
	public:
		/* Initializes a new instance of a skybox renderer. */
		SkyboxRenderer(_In_ GraphicsAdapter *device);
		SkyboxRenderer(_In_ const SkyboxRenderer &value) = delete;
		SkyboxRenderer(_In_ SkyboxRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~SkyboxRenderer(void);

		_Check_return_ SkyboxRenderer& operator =(_In_ const SkyboxRenderer &other) = delete;
		_Check_return_ SkyboxRenderer& operator =(_In_ SkyboxRenderer &&other) = delete;

		/* Render the specified cube map to the screen. */
		void Render(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ const Texture *skybox);

	private:
		Uniform *matView, *matProj, *texture;
		Attribute *pos;
		Buffer *vbo;

		GraphicsAdapter *device;

		void Begin(const Matrix &view, const Matrix &proj);
		void End(void);
		void InitShader(void);
	};
}