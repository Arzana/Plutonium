#include "Graphics\Portals\PortalRenderer.h"
#include "GameLogic\EuclidRoom.h"

Plutonium::PortalRenderer::PortalRenderer(GraphicsAdapter * device, const char * vrtxShdr)
	: device(device), INIT_BUS(OnRoomRender)
{
	/* Create shader. */
	shdr = Shader::FromFile(vrtxShdr);

	/* Get uniforms. */
	matMdl = shdr->GetUniform("u_model");
	matView = shdr->GetUniform("u_view");
	matProj = shdr->GetUniform("u_projection");

	/* Get attribute. */
	pos = shdr->GetAttribute("a_position");
}

Plutonium::PortalRenderer::~PortalRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::PortalRenderer::Render(const Matrix & view, const Matrix & proj, Tree<PortalRenderArgs>* portals)
{
	/* Temporary argument buffer. */
	Tree<SceneRenderArgs> result;
	portals->CloneStructure(&result);

	/* Navigate to the bottom of the tree. */
	while (!portals->IsStump()) portals->ClimbDown();

	/* Create view and projection matrices for all portals. */
	RecursiveCreateMatrices(view, proj, portals, &result);

	/* Navigate to the top of the tree. */
	while (result.HasBranch()) result.ClimbUp();

	/* Stencil render the scene. */
	BeginStencil();
	RecursiveRenderPortals(&result);
	EndStencil();

	/* Normal render the scene. */
	RecursiveRenderScene(&result);

	/* Disable stencil.*/
	device->SetStencilTest(DepthState::None);
}

void Plutonium::PortalRenderer::RecursiveCreateMatrices(const Matrix & view, const Matrix & proj, Tree<PortalRenderArgs>* portals, Tree<SceneRenderArgs>* result)
{
	/* Get current values. */
	PortalRenderArgs &portal = portals->Value();
	SceneRenderArgs &scene = result->Value();

	/* Populate result. */
	scene.View = view;
	scene.Projection = proj;
	scene.Portal = portal.Portal;
	scene.SceneID = portal.Portal->Destination->GetID();

	/* if branch has children move up. */
	if (portals->HasBranch())
	{
		portals->ClimbUp();
		result->ClimbUp();

		/* Create new matrices. */
		Matrix newView = portal.Portal->GetInverseView(view);
		Matrix newProj = portal.Portal->GetClippedProjection(view, proj);

		/* Recursive up the tree. */
		RecursiveCreateMatrices(newView, newProj, portals, result);
	}

	/* Run for all neighbors. */
	while (portals->NextBranch())
	{
		result->NextBranch();
		RecursiveCreateMatrices(view, proj, portals, result);
	}

	/* Make sure we move back one, once the tree limit is reached. */
	if (!portals->IsStump())
	{
		portals->ClimbDown();
		result->ClimbDown();
	}
}

void Plutonium::PortalRenderer::BeginStencil(void)
{
	/* Start stencil shader. */
	shdr->Begin();

	/* Disable rendering back side of the portal. */
	device->SetFaceCull(FaceCullState::Back);

	/* Disable color and depth output. */
	device->SetColorOutput(false, false, false, false);
	device->SetDepthOuput(false);

	/* Set stencil to always fail and enable write. */
	device->SetStencilOuput(0xFF);
	device->SetStencilFailOperation(StencilOperation::Replace);

	/* Clean stencil from previous render. */
	device->Clear(ClearTarget::Stencil);
}

void Plutonium::PortalRenderer::RecursiveRenderPortals(Tree<SceneRenderArgs>* result)
{
	/* Get current render argument. */
	SceneRenderArgs curScene = result->Value();

	/* Render portal frame. */
	RenderPortalFrameStencil(&curScene);

	/* Render neighbor is available. */
	if (result->NextBranch()) RecursiveRenderPortals(result);

	/* Render parent. */
	if (!result->IsStump())
	{
		result->ClimbDown();
		RecursiveRenderPortals(result);
	}
}

void Plutonium::PortalRenderer::RenderPortalFrameStencil(SceneRenderArgs * scene)
{
	/* Update stencil refrence. */
	device->SetStencilTest(DepthState::Never, scene->SceneID);

	/* Render portal frame. */
	RenderPortalFrame(scene->View, scene->Projection, scene->Portal);
}

void Plutonium::PortalRenderer::RenderPortalFrameDepth(SceneRenderArgs * scene)
{
	/* Restart shader and disable color output. */
	device->SetColorOutput(false, false, false, false);
	device->SetDepthTest(DepthState::Always);
	shdr->Begin();

	/* Render portal frame. */
	RenderPortalFrame(scene->View, scene->Projection, scene->Portal);

	/* End shader and enable color output again. */
	shdr->End();
	device->SetDepthTest(DepthState::LessOrEqual);
	device->SetColorOutput(true, true, true, true);
}

void Plutonium::PortalRenderer::RenderPortalFrame(const Matrix & view, const Matrix & proj, Portal * portal)
{
	/* Set uniforms. */
	matMdl->Set(portal->GetWorld());
	matView->Set(view);
	matProj->Set(proj);

	/* Set vertex attribute. */
	Buffer *buffer = portal->mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render portal frame. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
}

void Plutonium::PortalRenderer::RecursiveRenderScene(Tree<SceneRenderArgs>* portals)
{
	/* Render current scene. */
	SceneRenderArgs &scene = portals->Value();
	RenderScene(&scene);

	/* Render childs. */
	if (portals->HasBranch())
	{
		portals->ClimbUp();
		RecursiveRenderScene(portals);
	}

	/* Render neighbors. */
	while (portals->NextBranch()) RecursiveRenderScene(portals);

	/* Make sure we move back one, once the tree limit is reached. */
	if (!portals->IsStump()) portals->ClimbDown();
}

void Plutonium::PortalRenderer::RenderScene(SceneRenderArgs * scene)
{
	/* Update stencil refrence. */
	device->SetStencilTest(DepthState::Equal, scene->SceneID);

	/* Allow user to render the scene. */
	OnRoomRender.Post(this, *scene);

	/* Render the portals frame to the depth buffer to make sure we don't see portals through walls. */
	RenderPortalFrameDepth(scene);
}

void Plutonium::PortalRenderer::EndStencil(void)
{
	/* End stencil shader. */
	shdr->End();

	/* Disable stencil output and enable color and depth output. */
	device->SetStencilOuput(0x00);
	device->SetColorOutput(true, true, true, true);
	device->SetDepthOuput(true);
}