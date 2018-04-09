#include "GameLogic\EuclidRoom.h"
#include "Streams\FileReader.h"
#include "Graphics\Models\ObjLoader.h"
#include "Graphics\Portals\PobjLoader.h"
#include "Core\StringFunctions.h"

using namespace Plutonium;

int32 IdCnt = 1;

Plutonium::EuclidRoom::~EuclidRoom(void)
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

std::vector<EuclidRoom*> Plutonium::EuclidRoom::FromFile(const char * path)
{
	std::vector<EuclidRoom*> result;

	/* Load raw data. */
	FileReader reader(path, true);
	const PobjLoaderResult *raw = _CrtLoadPobjMtl(path);

	/* Check if file load has been successful. */
	if (!raw->Successful)
	{
		LOG_WAR("Tinyobj loading log:\n%s", raw->Log);
		LOG_THROW("Unable to load model '%s'!", reader.GetFileName());
		delete_s(raw);
		return result;
	}

	/* Load individual rooms. */
	std::vector<int> didx;
	for (size_t i = 0; i < raw->Rooms.size(); i++)
	{
		tinyobj::room_t room = raw->Rooms.at(i);

		/* Create current room. */
		EuclidRoom *cur = new EuclidRoom();
		cur->id = IdCnt++;
		float gx = raw->Vertices.normals.at(3 * room.gravity);
		float gy = raw->Vertices.normals.at(3 * room.gravity + 1);
		float gz = raw->Vertices.normals.at(3 * room.gravity + 2);
		cur->gravityForce = Vector3(gx, gy, gz);

		/* Load shapes. */
		for (size_t j = 0; j < room.shapes.size(); j++)
		{
			tinyobj::shape_t shape = room.shapes.at(j);

			/* Load mesh. */
			Mesh *mesh = Mesh::RFromFile(raw, i, j);

			/* Get correct material. */
			tinyobj::material_t mtl = shape.mesh.material_ids.at(0) != -1 ? raw->Materials.at(static_cast<size_t>(shape.mesh.material_ids.at(0))) : _CrtGetDefMtl();
			if (mtl.diffuse_texname.length())
			{
				char mtlPath[FILENAME_MAX];
				mrgstr(reader.GetFileDirectory(), mtl.diffuse_texname.c_str(), mtlPath);
				Texture *texture = Texture::FromFile(mtlPath);

				/* Add shape to the model. */
				cur->shapes.push_back(new Shape(mesh, texture));
			}
			else LOG_WAR("Skipping mesh '%s', material '%s'(%zu), diffuse texture not specified!", mesh->Name, mtl.name.c_str(), shape.mesh.material_ids.at(0));
		}

		/* Load portals. */
		for (size_t j = 0; j < room.portals.size(); j++)
		{
			Portal *portal = new Portal(Mesh::PFromFile(raw, i, j));
			cur->portals.push_back(portal);
			didx.push_back(room.portals.at(j).destination);
		}

		cur->Finalize();	// TODO: remove bullshit
		result.push_back(cur);
	}

	for (size_t i = 0, k = 0; i < result.size(); i++)
	{
		EuclidRoom *room = result.at(i);

		/* Set portal destinations. */
		for (size_t j = 0; j < room->portals.size(); j++, k++)
		{
			room->portals.at(j)->Destination = result.at(didx.at(k));
		}
	}

	LOG("Finished loading map: '%s', %d rooms, %d portals.", reader.GetFileName(), result.size(), didx.size());
	return result;
}

void Plutonium::EuclidRoom::AddPortals(Tree<PortalRenderArgs>* portals) const
{
	for (size_t i = 0; i < this->portals.size(); i++)
	{
		portals->Add({ id, this->portals.at(i) });
	}
}

void Plutonium::EuclidRoom::SetScale(float scale)
{
	WorldObject::SetScale(scale);
	for (size_t i = 0; i < portals.size(); i++) portals.at(i)->SetScale(scale);
}

void Plutonium::EuclidRoom::Finalize(void)
{
	for (size_t i = 0; i < shapes.size(); i++)
	{
		shapes.at(i)->Mesh->Finalize();
	}
}