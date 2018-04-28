#include "Graphics\Models\StaticModel.h"
#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"

using namespace Plutonium;

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

StaticModel * Plutonium::StaticModel::FromFile(const char * path, AssetLoader *loader)
{
	/* Load raw data. */
	FileReader reader(path, true);
	const ObjLoaderResult *raw = _CrtLoadObjMtl(path);

	/* Load individual shapes. */
	StaticModel *result = new StaticModel(loader->GetWindow());
	result->path = heapstr(path);
	result->name = heapstr(reader.GetFileNameWithoutExtension());
	for (size_t i = 0; i < raw->Shapes.size(); i++)
	{
		/* Get current mesh and associated material. */
		const ObjLoaderMesh &shape = raw->Shapes.at(i);
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
			result->shapes.push_back(new PhongShape(mesh, &material, loader));
		}
	}

	/* Finalize loading. */
	result->Finalize();
	LOG("Finished loading model '%s', %zu/%zu distinct materials.", reader.GetFileNameWithoutExtension(), result->shapes.size(), raw->Materials.size());
	delete_s(raw);
	return result;
}

void Plutonium::StaticModel::Finalize(void)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		shapes.at(i)->Mesh->Finalize(wnd);
	}
}

int64 Plutonium::StaticModel::ContainsMaterial(const char * name)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		PhongShape *cur = shapes.at(i);
		if (!strcmp(cur->MaterialName, name)) return i;
	}

	return -1;
}