#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Native\Buffer.h"
#include "Graphics\GraphicsAdapter.h"

namespace Plutonium
{
	struct SkyboxRenderer
	{
		SkyboxRenderer(_In_ GraphicsAdapter *device, _In_ const char *vrtxShdr, _In_ const char *fragShdr);
		SkyboxRenderer(_In_ const SkyboxRenderer &value) = delete;
		SkyboxRenderer(_In_ SkyboxRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~SkyboxRenderer(void);

		_Check_return_ SkyboxRenderer& operator =(_In_ const SkyboxRenderer &other) = delete;
		_Check_return_ SkyboxRenderer& operator =(_In_ SkyboxRenderer &&other) = delete;

		void Render(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ const Texture *skybox);

	private:
		Shader *shdr;
		Uniform *matView, *matProj, *texture;
		Attribute *pos;
		Buffer *vbo;

		GraphicsAdapter *device;

		void Begin(const Matrix &view, const Matrix &proj);
		void End(void);
	};
}