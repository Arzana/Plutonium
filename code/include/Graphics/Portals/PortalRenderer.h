#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Portals\Portal.h"
#include "Graphics\GraphicsAdapter.h"

namespace Plutonium
{
	/* Defines a function that can be used to render a scene. */
	using SceneRenderFunc = void(*)(const Matrix &view, const Matrix &projection);

	struct PortalRenderer
	{
		/* Initializes a new instance of a basic model renderer. */
		PortalRenderer(_In_ GraphicsAdapter *device, _In_ const char *vrtxShdr, _In_ const char * fragShdr);
		PortalRenderer(_In_ const PortalRenderer &value) = delete;
		PortalRenderer(_In_ PortalRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~PortalRenderer(void);

		_Check_return_ PortalRenderer& operator =(_In_ const PortalRenderer &other) = delete;
		_Check_return_ PortalRenderer& operator =(_In_ PortalRenderer &&other) = delete;

		/* Sets the function that can be used to render the scene. */
		inline void SetSceneRenderFunc(_In_ SceneRenderFunc func)
		{
			drawNonPortals = func;
		}
		/* All the specified portals. */
		void Render(const Matrix &view, const Matrix &proj);

	private:
		size_t maxRecursionDepth;
		std::vector<Portal*> portals;
		SceneRenderFunc drawNonPortals;

		GraphicsAdapter *device;
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj;
		Attribute *pos;

		void Begin(void);
		void RenderPortals(const Matrix &view, const Matrix &proj, int32 curRecusrionDepth);
		void RenderSinglePortal(Portal *portal, const Matrix &view, const Matrix &proj);
		void End(void);
	};
}