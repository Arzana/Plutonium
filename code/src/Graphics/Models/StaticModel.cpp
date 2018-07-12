#include "Graphics\Models\StaticModel.h"
#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"

using namespace Plutonium;

Plutonium::StaticModel::StaticModel(PhongMaterial * material)
	: wnd(nullptr), name(heapstr(material->MaterialName)), path(heapstr(""))
{
	shapes.push_back(material);
}

Plutonium::StaticModel::~StaticModel(void)
{
	free_s(name);
	free_s(path);

	/* Free underlying shapes. */
	while (shapes.size() > 0)
	{
		delete_s(shapes.back());
		shapes.pop_back();
	}
}

void Plutonium::StaticModel::Finalize(void)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		shapes.at(i)->Mesh->Finalize(wnd);
	}
}

StaticModel * Plutonium::StaticModel::FromFile(const char * path, AssetLoader * loader, std::atomic<float>* progression)
{
	/* Load raw data. */
	FileReader reader(path, true);
	const ObjLoaderResult *raw = _CrtLoadObjMtl(path, progression, 0.7f);

	/* Load individual shapes. */
	StaticModel *result = new StaticModel(loader->GetWindow());
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());
	for (size_t i = 0; i < raw->Shapes.size(); i++)
	{
		/* Get current mesh and associated material, skip any mesh that does not define any indices. */
		const ObjLoaderMesh &shape = raw->Shapes.at(i);
		if (shape.Indices.size() > 0)
		{
			const ObjLoaderMaterial &material = shape.Material != -1 ? raw->Materials.at(shape.Material) : ObjLoaderMaterial();

			/* Create final mesh. */
			Mesh *mesh = Mesh::FromFile(raw, i);

			/* Check if we can merge the mesh into another one to have on draw calls. */
			int64 j = result->ContainsMaterial(material.Name);
			if (j != -1)
			{
				result->shapes.at(j)->Mesh->Append(mesh);
				delete_s(mesh);
			}
			else
			{
				/* Push material to shapes. */
				result->shapes.push_back(new PhongMaterial(mesh, &material, loader));
			}
		}
		else LOG_WAR("Shape '%s' in model '%s' does not define any vertices, skipping mesh!", shape.Name, result->name);

		if (progression) progression->store(0.7f + (static_cast<float>(i) / static_cast<float>(raw->Shapes.size())) * 0.3f);
	}

	/* Finalize loading. */
	result->Finalize();
	LOG("Finished loading model '%s', %zu/%zu distinct materials.", reader.GetFileNameWithoutExtension(), result->shapes.size(), raw->Materials.size());
	delete_s(raw);
	return result;
}

int64 Plutonium::StaticModel::ContainsMaterial(const char * name)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		PhongMaterial *cur = shapes.at(i);
		if (!strcmp(cur->MaterialName, name)) return i;
	}

	return -1;
}