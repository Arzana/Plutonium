#include "Graphics\Diagnostics\DebugMeshRenderer.h"
#include "Graphics\Materials\MaterialBP.h"

Plutonium::DebugMeshRenderer::DebugMeshRenderer(GraphicsAdapter * device, DebuggableValues mode)
	: mode(mode)
{
	wfrenderer = new WireframeRenderer();
	nrenderer = new NormalRenderer();
	ulrenderer = new UnlitRenderer();
	lrenderer = new LightingRenderer(device);

	defBmpMap = new Texture(1, 1, device->GetWindow(), &TextureCreationOptions::DefaultNoMipMap, "default bump");
	defBmpMap->SetData(Color::Malibu.ToArray());

	defAlphaMap = new Texture(1, 1, device->GetWindow(), &TextureCreationOptions::DefaultNoMipMap, "default alpha");
	defAlphaMap->SetData(Color::White.ToArray());
}

Plutonium::DebugMeshRenderer::~DebugMeshRenderer(void)
{
	delete_s(wfrenderer);
	delete_s(nrenderer);
	delete_s(ulrenderer);
	delete_s(lrenderer);

	delete_s(defBmpMap);
	delete_s(defAlphaMap);
}

void Plutonium::DebugMeshRenderer::Render(const Matrix & view, const Matrix & proj, Vector3 camPos)
{
	switch (mode)
	{
	case DebuggableValues::Wireframe:
		wfrenderer->Begin(view, proj);
		for (size_t i = 0; i < sModels.size(); i++) RenderWfStatic(sModels.at(i));
		for (size_t i = 0; i < dModels.size(); i++) RenderWfDynamic(dModels.at(i));
		wfrenderer->End();
		break;
	case DebuggableValues::Normals:
		nrenderer->Begin(view, proj);
		for (size_t i = 0; i < sModels.size(); i++) RenderNStatic(sModels.at(i));
		for (size_t i = 0; i < dModels.size(); i++) RenderNDynamic(dModels.at(i));
		nrenderer->End();
		break;
	case DebuggableValues::Unlit:
		ulrenderer->Begin(view, proj);
		for (size_t i = 0; i < sModels.size(); i++) RenderUlStatic(sModels.at(i));
		for (size_t i = 0; i < dModels.size(); i++) RenderUlDynamic(dModels.at(i));
		ulrenderer->End();
		break;
	case DebuggableValues::Lighting:
		lrenderer->BeginModels(view, proj);
		for (size_t i = 0; i < sModels.size(); i++) RenderLStatic(sModels.at(i));
		for (size_t i = 0; i < dModels.size(); i++) RenderLDynamic(dModels.at(i));
		if (dLights.size() > 0)
		{
			lrenderer->BeginDirectionalLights(camPos);
			while (dLights.size() > 0)
			{
				RenderLDLight(dLights.front());
				dLights.pop();
			}
		}

		if (pLights.size() > 0)
		{
			lrenderer->BeginPointLights();
			while (pLights.size() > 0)
			{
				RenderLPLight(pLights.front());
				pLights.pop();
			}
		}

		lrenderer->End();
		break;
	}

	sModels.clear();
	dModels.clear();
	while (dLights.size() > 0) dLights.pop();
	while (pLights.size() > 0) pLights.pop();
}

void Plutonium::DebugMeshRenderer::RenderWfStatic(const StaticObject * model)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		StaticModel::Shape cur = underlying->shapes.at(i);
#if defined (DEBUG)
		Color clr = cur.Material->Debug;
#else
		Color clr = Color::Red;
#endif
		wfrenderer->Render(model->GetWorld(), underlying->shapes.at(i).Mesh, clr);
	}
}

void Plutonium::DebugMeshRenderer::RenderWfDynamic(const DynamicObject * model)
{
	wfrenderer->Render(model->GetWorld(), model->GetCurrentFrame(), Color::Yellow);
}

void Plutonium::DebugMeshRenderer::RenderNStatic(const StaticObject * model)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		StaticModel::Shape cur = underlying->shapes.at(i);
		nrenderer->Render(model->GetWorld(), cur.Mesh, cur.Material->Normal);
	}
}

void Plutonium::DebugMeshRenderer::RenderNDynamic(const DynamicObject * model)
{
	nrenderer->Render(model->GetWorld(), model->GetCurrentFrame(), defBmpMap);
}

void Plutonium::DebugMeshRenderer::RenderUlStatic(const StaticObject * model)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		StaticModel::Shape cur = underlying->shapes.at(i);
		ulrenderer->Render(model->GetWorld(), cur.Mesh, cur.Material->Diffuse, cur.Material->Opacity);
	}
}

void Plutonium::DebugMeshRenderer::RenderUlDynamic(const DynamicObject * model)
{
	ulrenderer->Render(model->GetWorld(), model->GetCurrentFrame(), model->model->skin, defAlphaMap);
}

void Plutonium::DebugMeshRenderer::RenderLStatic(const StaticObject * model)
{
	const StaticModel *underlying = model->GetModel();
	for (size_t i = 0; i < underlying->shapes.size(); i++)
	{
		StaticModel::Shape cur = underlying->shapes.at(i);
		lrenderer->Render(model->GetWorld(), cur.Mesh);
	}
}

void Plutonium::DebugMeshRenderer::RenderLDynamic(const DynamicObject * model)
{
	lrenderer->Render(model->GetWorld(), model->GetCurrentFrame());
}

void Plutonium::DebugMeshRenderer::RenderLDLight(const DirectionalLight * light)
{
	for (size_t i = 0; i < sModels.size(); i++)
	{
		const StaticObject *object = sModels.at(i);
		const StaticModel *model = object->GetModel();
		for (size_t j = 0; j < model->shapes.size(); j++)
		{
			StaticModel::Shape cur = model->shapes.at(j);
			lrenderer->Render(object->GetWorld(), cur.Mesh, cur.Material->Normal, light);
		}
	}

	for (size_t i = 0; i < dModels.size(); i++)
	{
		const DynamicObject *object = dModels.at(i);
		lrenderer->Render(object->GetWorld(), object->GetCurrentFrame(), defBmpMap, light);
	}
}

void Plutonium::DebugMeshRenderer::RenderLPLight(const PointLight * light)
{
	for (size_t i = 0; i < sModels.size(); i++)
	{
		const StaticObject *object = sModels.at(i);
		const StaticModel *model = object->GetModel();
		for (size_t j = 0; j < model->shapes.size(); j++)
		{
			StaticModel::Shape cur = model->shapes.at(j);
			lrenderer->Render(object->GetWorld(), cur.Mesh, cur.Material->Normal, light);
		}
	}

	for (size_t i = 0; i < dModels.size(); i++)
	{
		const DynamicObject *object = dModels.at(i);
		lrenderer->Render(object->GetWorld(), object->GetCurrentFrame(), defBmpMap, light);
	}
}