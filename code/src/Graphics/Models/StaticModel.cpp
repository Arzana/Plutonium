#include "Graphics\Models\StaticModel.h"
#include "Content\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"

using namespace Plutonium;

Plutonium::StaticModel::~StaticModel(void)
{
	/* Free underlying shapes. */
	while (shapes.size() > 0)
	{
		delete_s(shapes.back());
		shapes.pop_back();
	}
}

StaticModel * Plutonium::StaticModel::FromFile(const char * path)
{
	/* Load raw data. */
	FileReader reader(path, true);
	const ObjLoaderResult *raw = _CrtLoadObjMtl(path);

	/* Load individual shapes. */
	StaticModel *result = new StaticModel();
	for (size_t i = 0; i < raw->Shapes.size(); i++)
	{
		/* Get current mesh and associated material. */
		const ObjLoaderMesh &shape = raw->Shapes.at(i);
		const ObjLoaderMaterial &material = shape.Material != -1 ? raw->Materials.at(shape.Material) : ObjLoaderMaterial();

		/* Create final mesh and texture if able to. */
		if (strlen(material.DiffuseMap.Path) > 0 && strlen(material.AmbientMap.Path) > 0)
		{
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
				result->shapes.push_back(new PhongShape(mesh, &material));
			}
		}
		else LOG_WAR("Skipping material '%s'(%zu), ambient or diffuse texture not specified!", material.Name, shape.Material);
	}

	/* Finalize loading. */
	result->Finalize();
	delete_s(raw);
	return result;
}

void Plutonium::StaticModel::Finalize(void)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		shapes.at(i)->Mesh->Finalize();
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