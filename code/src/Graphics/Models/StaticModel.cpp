#include "Graphics\Models\StaticModel.h"
#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"
#include "Graphics\Materials\MaterialBP.h"

#if defined (DEBUG)
#include "Core\Stopwatch.h"
#endif

constexpr bool PRE_SORT_MATERIALS = true;
constexpr bool RECALC_NORMALS = false;

using namespace Plutonium;

bool MaterialSortPredicate(const ObjLoaderMesh &a, const ObjLoaderMesh &b)
{
	return a.Material > b.Material;
}

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
		Mesh *cur = shapes.at(i).Mesh;
		if (!cur->buffer) cur->Finalize(wnd);
	}
}

StaticModel * Plutonium::StaticModel::FromFile(const char * path, AssetLoader * loader, std::atomic<float> * progression)
{
	constexpr float MOD = 0.7f;
#if defined (DEBUG)
	Stopwatch sw = Stopwatch::StartNew();
#endif

	/* Load raw data. */
	FileReader reader(path, true);
	ObjLoaderResult *raw = _CrtLoadObjMtl(path, progression, MOD);
	if (PRE_SORT_MATERIALS) std::sort(raw->Shapes.begin(), raw->Shapes.end(), MaterialSortPredicate);

	/* Load individual shapes. */
	StaticModel *result = new StaticModel(loader->GetWindow());
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());

	float initialShapeCnt = recip(static_cast<float>(raw->Shapes.size()));
	float loaded = 0.0f;

	const ObjLoaderMaterial &defMaterial = raw->GetDefaultMaterial() != -1 ? raw->Materials.at(raw->GetDefaultMaterial()) : ObjLoaderMaterial();
	int64 oldMaterial = -2;
	while (raw->Shapes.size() > 0)
	{
		/* Get current mesh and associated material, skip any mesh that does not define any indices. */
		const ObjLoaderMesh &shape = raw->Shapes.back();
		if (shape.Indices.size() > 0)
		{
			const ObjLoaderMaterial &material = shape.Material != -1 ? raw->Materials.at(shape.Material) : defMaterial;

			/* Create final mesh. */
			Mesh *mesh = Mesh::FromFile(raw, raw->Shapes.size() - 1, RECALC_NORMALS);

			/* Check if we can merge the mesh into another one to have on draw calls. */
			int64 j = result->ContainsMaterial(material.Name);
			if (j != -1)
			{
				result->shapes.at(j).Mesh->Append(mesh);
				delete_s(mesh);
			}
			else
			{
				/* Finalize the mesh if available. */
				if (PRE_SORT_MATERIALS && (oldMaterial == -2 || oldMaterial != shape.Material))
				{
					if (oldMaterial != -2) result->shapes.back().Mesh->Finalize(result->wnd);
					oldMaterial = shape.Material;
				}

				/* Push material to shapes. */
				result->shapes.push_back({ new MaterialBP(&material, loader), mesh });
			}
		}
		else LOG_WAR("Shape '%s' in model '%s' does not define any vertices, skipping mesh!", shape.Name, result->name);

		raw->Shapes.pop_back();
		if (progression) progression->store(MOD + (loaded++ * initialShapeCnt) * (1.0f - MOD));
	}

	/* Finalize the final meshes. */
	result->Finalize();

	/* Finalize loading. */
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