#include "Graphics\Portals\PortalRenderer.h"
#include "GameLogic\EuclidRoom.h"

Plutonium::PortalRenderer::PortalRenderer(GraphicsAdapter * device, const char * vrtxShdr)
	: device(device), INIT_BUS(OnBeginRoomRender), INIT_BUS(OnRoomRender), INIT_BUS(OnEndRoomRender)
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

	/* navigate to the top of the tree. */
	while (portals->HasBranch()) portals->ClimbUp();

	/* Stencil render the scene. */
	BeginStencil();
	RecursiveRenderPortals(portals, &result);
	EndStencil();

	/* Normal render the scene. */
	OnBeginRoomRender.Post(this, EventArgs());
	RecursiveRenderScene(&result);
	OnEndRoomRender.Post(this, EventArgs());

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

void Plutonium::PortalRenderer::RecursiveRenderPortals(Tree<PortalRenderArgs>* portals, Tree<SceneRenderArgs>* result)
{
	/* Get current portal and render argument. */
	PortalRenderArgs curPortal = portals->Value();
	SceneRenderArgs curScene = result->Value();

	/* Render portal frame. */
	RenderPortalFrame(curScene.View, curScene.Projection, &curPortal);

	/* Render neighbor is available. */
	if (portals->NextBranch()) RecursiveRenderPortals(portals, result);

	/* Render parent. */
	if (!portals->IsStump())
	{
		portals->ClimbDown();
		RecursiveRenderPortals(portals, result);
	}
}

void Plutonium::PortalRenderer::RenderPortalFrame(const Matrix & view, const Matrix & proj, PortalRenderArgs * portal)
{
	/* Update stencil refrence. */
	device->SetStencilTest(DepthState::Never, portal->Portal->Destination->GetID());

	/* Set uniforms. */
	matMdl->Set(portal->Portal->GetWorld());
	matView->Set(view);
	matProj->Set(proj);

	/* Set vertex attribute. */
	Buffer *buffer = portal->Portal->mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render portal frame to stencil buffer. */
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
}

void Plutonium::PortalRenderer::EndStencil(void)
{
	/* End stencil shader. */
	shdr->End();

	/* Disable stencil output and enable color and depth output. */
	device->SetStencilOuput(0x00);
	device->SetColorOutput(true, true, true, true);
	device->SetDepthOuput(true);
	device->SetDepthTest(DepthState::Always);
}