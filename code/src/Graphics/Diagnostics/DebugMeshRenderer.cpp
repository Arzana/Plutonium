#include "Graphics\Diagnostics\DebugMeshRenderer.h"

Plutonium::DebugMeshRenderer::DebugMeshRenderer(WindowHandler wnd, DebuggableValues mode)
	: mode(mode)
{
	wfrenderer = new WireframeRenderer();
	nrenderer = new NormalRenderer();

	defBmpMap = new Texture(1, 1, wnd, 0, "default");
	defBmpMap->SetData(Color::Malibu.ToArray());
}

Plutonium::DebugMeshRenderer::~DebugMeshRenderer(void)
{
	delete_s(wfrenderer);
	delete_s(nrenderer);
	delete_s(defBmpMap);
}

void Plutonium::DebugMeshRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	switch (mode)
	{
	case DebuggableValues::Wireframe:
		wfrenderer->Begin(view, proj);
		break;
	case DebuggableValues::Normals:
		nrenderer->Begin(view, proj);
		break;
	default:
		LOG_THROW("Begin not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::Render(const StaticObject *model, Color color)
{
	switch (mode)
	{
	case DebuggableValues::Wireframe:
		RenderWfStatic(model, color);
		break;
	case DebuggableValues::Normals:
		RenderNStatic(model);
		break;
	default:
		LOG_THROW("Render not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::Render(const DynamicObject * model, Color color)
{
	switch (mode)
	{
	case DebuggableValues::Wireframe:
		RenderWfDynamic(model, color);
		break;
	case DebuggableValues::Normals:
		RenderNDynamic(model);
		break;
	default:
		LOG_THROW("Render not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::End(void)
{
	/* Make sure end isn't called twice. */
	switch (mode)
	{
	case DebuggableValues::Wireframe:
		wfrenderer->End();
		break;
	case DebuggableValues::Normals:
		nrenderer->End();
		break;
	default:
		LOG_THROW("End not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::RenderWfStatic(const StaticObject * model, Color color)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		wfrenderer->Render(model->GetWorld(), underlying->shapes.at(i)->Mesh, color);
	}
}

void Plutonium::DebugMeshRenderer::RenderWfDynamic(const DynamicObject * model, Color color)
{
	wfrenderer->Render(model->GetWorld(), model->GetCurrentFrame(), color);
}

void Plutonium::DebugMeshRenderer::RenderNStatic(const StaticObject * model)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		PhongMaterial *cur = underlying->shapes.at(i);
		nrenderer->Render(model->GetWorld(), cur->Mesh, cur->BumpMap);
	}
}

void Plutonium::DebugMeshRenderer::RenderNDynamic(const DynamicObject * model)
{
	nrenderer->Render(model->GetWorld(), model->GetCurrentFrame(), defBmpMap);
}