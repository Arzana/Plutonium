#include "Graphics\Models\StaticModel.h"
#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"
#include "Graphics\Materials\MaterialBP.h"

constexpr bool ALLOW_MESH_MERGES = true;

using namespace Plutonium;

Plutonium::StaticModel::StaticModel(MaterialBP * material, Mesh * mesh)
	: wnd(nullptr), name(heapstr(material->Name)), path(heapstr(""))
{
	shapes.push_back({ material, mesh });
}

Plutonium::StaticModel::~StaticModel(void)
{
	free_s(name);
	free_s(path);

	/* Free underlying shapes. */
	while (shapes.size() > 0)
	{
		Shape cur = shapes.back();
		delete_s(cur.Material);
		delete_s(cur.Mesh);
		shapes.pop_back();
	}
}

void Plutonium::StaticModel::Finalize(void)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		shapes.at(i).Mesh->Finalize(wnd);
	}
}

#include "Core\Stopwatch.h"

StaticModel * Plutonium::StaticModel::FromFile(const char * path, AssetLoader * loader, std::atomic<float>* progression)
{
	Stopwatch sw = Stopwatch::StartNew();

	/* Load raw data. */
	FileReader reader(path, true);
	ObjLoaderResult *raw = _CrtLoadObjMtl(path, progression, 0.7f);

	/* Load individual shapes. */
	StaticModel *result = new StaticModel(loader->GetWindow());
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());
	
	float initialShapeCnt = recip(static_cast<float>(raw->Shapes.size()));
	float loaded = 0.0f;
	while (raw->Shapes.size() > 0)
	{
		/* Get current mesh and associated material, skip any mesh that does not define any indices. */
		const ObjLoaderMesh &shape = raw->Shapes.back();
		if (shape.Indices.size() > 0)
		{
			const ObjLoaderMaterial &material = shape.Material != -1 ? raw->Materials.at(shape.Material) : ObjLoaderMaterial();

			/* Create final mesh. */
			Mesh *mesh = Mesh::FromFile(raw, raw->Shapes.size() - 1);

			/* Check if we can merge the mesh into another one to have on draw calls. */
			int64 j = result->ContainsMaterial(material.Name);
			if (j != -1 && ALLOW_MESH_MERGES)
			{
				result->shapes.at(j).Mesh->Append(mesh);
				delete_s(mesh);
			}
			else
			{
				/* Push material to shapes. */
				result->shapes.push_back({ new MaterialBP(&material, loader), mesh });
			}
		}
		else LOG_WAR("Shape '%s' in model '%s' does not define any vertices, skipping mesh!", shape.Name, result->name);

		raw->Shapes.pop_back();
		if (progression) progression->store(0.7f + (loaded++ * initialShapeCnt) * 0.3f);
	}

	/* Finalize loading. */
	result->Finalize();
	LOG("Finished loading model '%s', %zu/%zu distinct materials, took %f seconds.", reader.GetFileNameWithoutExtension(), result->shapes.size(), raw->Materials.size(), static_cast<float>(sw.Milliseconds()) * 0.001f);
	delete_s(raw);
	return result;
}

int64 Plutonium::StaticModel::ContainsMaterial(const char * name)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		if (eqlstr(shapes.at(i).Material->Name, name)) return i;
	}

	return -1;
}