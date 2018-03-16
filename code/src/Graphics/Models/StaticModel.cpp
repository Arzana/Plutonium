#include "Graphics\Models\StaticModel.h"
#include "Graphics\Models\ObjLoader.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\StringFunctions.h"

using namespace Plutonium;

Plutonium::StaticModel::~StaticModel(void)
{
	/* Free underlying shapes. */
	while (shapes.size() > 0)
	{
		Shape *cur = shapes.back();
		delete_s(cur->Mesh);
		delete_s(cur->Material);
		delete_s(cur);
		shapes.pop_back();
	}
}

StaticModel * Plutonium::StaticModel::FromFile(const char * path)
{
	/* Load raw data. */
	FileReader reader(path);
	const ObjLoaderResult *raw = _CrtLoadObjMtl(path);

	/* Check if file load has been successful. */
	if (!raw->Successful)
	{
		LOG_WAR("TinyObj loading log:\n%s", raw->Log);
		LOG_THROW("Unable to load model '%s'!", reader.GetFileName());
		delete_s(raw);
		return nullptr;
	}

	StaticModel *result = new StaticModel();

	/* Load individual shapes. */
	for (size_t i = 0; i < raw->Shapes.size(); i++)
	{
		tinyobj::shape_t shape = raw->Shapes.at(i);

		/* Load mesh. */
		Mesh *mesh = Mesh::FromFile(raw, i);

		/* Get correct material. */
		tinyobj::material_t mtl = shape.mesh.material_ids.size() > 0 ? raw->Materials.at(static_cast<size_t>(shape.mesh.material_ids.at(0))) : _CrtGetDefMtl();
		char mtlPath[FILENAME_MAX];
		mrgstr(reader.GetFileDirectory(), mtl.diffuse_texname.c_str(), mtlPath);
		Texture *texture = Texture::FromFile(mtlPath);

		/* Add shape to the model. */
		result->shapes.push_back(new Shape(mesh, texture));
	}

	result->Finalize();	//TODO: Remove!

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
