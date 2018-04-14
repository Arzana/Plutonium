#pragma once
#include "Graphics\Rendering\Shader.h"
#include "Graphics\Portals\Portal.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Portals\SceneRenderArgs.h"
#include "Core\Collections\Tree.h"
#include "Core\Events\EventBus.h"

namespace Plutonium
{
	/* Defines a basic recursive portal renderer. */
	struct PortalRenderer
	{
		/* Occures when a room needs to be rendered. */
		EventBus<PortalRenderer, SceneRenderArgs> OnRoomRender;

		/* Initializes a new instance of a basic model renderer. */
		PortalRenderer(_In_ GraphicsAdapter *device, _In_ const char *vrtxShdr);
		PortalRenderer(_In_ const PortalRenderer &value) = delete;
		PortalRenderer(_In_ PortalRenderer &&value) = delete;
		/* Releases the resources allocated by the renderer. */
		~PortalRenderer(void);

		_Check_return_ PortalRenderer& operator =(_In_ const PortalRenderer &other) = delete;
		_Check_return_ PortalRenderer& operator =(_In_ PortalRenderer &&other) = delete;

		/* Renders the scene with the specified portals. */
		void Render(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ Tree<PortalRenderArgs> *portals);
		/* Renders the specified scenes with the specified portals. */
		inline void Render(_In_ const Matrix &view, _In_ const Matrix &proj, _In_ const std::vector<Tree<PortalRenderArgs>*> *portals)
		{
			for (size_t i = 0; i < portals->size(); i++) Render(view, proj, portals->at(i));
		}

	private:
		GraphicsAdapter *device;
		Shader *shdr;
		Uniform *matMdl, *matView, *matProj;
		Attribute *pos;

		void RecursiveCreateMatrices(const Matrix &view, const Matrix &proj, Tree<PortalRenderArgs> *portals, Tree<SceneRenderArgs> *result);
		void BeginStencil(void);
		void RecursiveRenderPortals(Tree<SceneRenderArgs> *result);
		void RenderPortalFrameStencil(SceneRenderArgs *scene);
		void RenderPortalFrameDepth(SceneRenderArgs *scene);
		void RenderPortalFrame(const Matrix &view, const Matrix &proj, Portal *portal);
		void RecursiveRenderScene(Tree<SceneRenderArgs> *portals);
		void RenderScene(SceneRenderArgs *scene);
		void EndStencil(void);
	};
}