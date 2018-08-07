#include "Graphics\Models\DynamicModel.h"
#include "Graphics\Models\Md2Loader.h"
#include "Streams\FileReader.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"
#include "Graphics\Materials\MaterialBP.h"
#include <cstring>

using namespace Plutonium;

Plutonium::DynamicModel::DynamicModel(AssetLoader *loader)
	: loader(loader)
{}

Plutonium::DynamicModel::~DynamicModel(void)
{
	free_s(name);
	free_s(path);
	delete_s(material);

	/* Release animation information. */
	for (size_t i = 0; i < animations.size(); i++) delete_s(animations.at(i));
	animations.clear();
}

void Plutonium::DynamicModel::Finalize(void)
{
	WindowHandler wnd = loader->GetWindow();

	/* Finalize all animation frames. */
	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationInfo *info = animations.at(i);
		for (size_t j = 0; j < info->Frames.size(); j++)
		{
			info->Frames.at(j)->Finalize(wnd);
		}
	}
}

DynamicModel * Plutonium::DynamicModel::FromFile(const char * path, AssetLoader *loader, const char * texture, std::atomic<float> * progression)
{
	constexpr float RAW_MOD = 0.1f;
	constexpr float MESH_MOD = 0.8f;

	/* Create result. */
	DynamicModel *result = new DynamicModel(loader);
	result->path = heapstr(path);

	/* Attempt to load raw data. */
	const Md2LoaderResult *raw = _CrtLoadMd2(path, progression, RAW_MOD);

	/* Check if there is at least one texture that can be used. */
	LOG_THROW_IF(raw->textures.size() < 1 && !texture, "No texture is defined for model '%s'!", result->name);

	/* Load all animation frames. */
	std::vector<Mesh*> meshes;
	for (size_t i = 0; i < raw->frames.size(); i++)
	{
		Mesh *mesh = Mesh::FromFile(raw, i, false);
		mesh->Finalize(loader->GetWindow()); // TODO: Remove!
		meshes.push_back(mesh);
		if (progression) progression->store(RAW_MOD + ((static_cast<float>(i) / raw->frames.size()) * MESH_MOD));
	}

	/* Parse frames to animations. */
	result->SplitFrames(meshes);

	/* Construct texture path. */
	FileReader reader(path, true);
	char tex[FILENAME_MAX];
	mrgstr(reader.GetFileDirectory(), texture ? texture : raw->textures.at(0), tex);

	/* Parse texture to result. */
	result->name = heapstr(reader.GetFileNameWithoutExtension());
	result->material = new MaterialBP(result->name, loader, nullptr, loader->LoadTexture(tex), nullptr, nullptr, nullptr);
	progression->store(1.0f);

	/* Log creation. */
	LOG("Finished loading model '%s' (%zu animation(s) with %zu total frames).", reader.GetFileName(), result->animations.size(), meshes.size());
	return result;
}

void Plutonium::DynamicModel::SplitFrames(std::vector<Mesh*> meshes)
{
	/* Handle all frames. */
	for (size_t i = 0; i < meshes.size(); i++)
	{
		Mesh *mesh = meshes.at(i);

		/* Get the name of the animation from the frame name. */
		size_t len = strlen(mesh->Name);
		char c = mesh->Name[len - 1];
		char parentAnimationName[64];

		/* Loop through frame name backwards untill the end if found. */
		for (size_t j = 1; c != '\0'; j++, c = mesh->Name[len - j])
		{
			/* Once the maxium for 2 numerical characters are read or the character is a letter. */
			if (isalpha(c) || j > 2)
			{
				/* Copy the animation name without the frame number. */
				substr(mesh->Name, 0, len - (j - 1), parentAnimationName);
				break;
			}
		}

		/* Check if the animation is already defined. */
		bool found = false;
		for (size_t j = 0; j < animations.size(); j++)
		{
			AnimationInfo *info = animations.at(j);
			if (!strcmp(info->Name, parentAnimationName))
			{
				/* If animation is already defined; add the mesh to the frame list. */
				found = true;
				info->Frames.push_back(mesh);
				break;
			}
		}

		/* If not found add as new animation. */
		if (!found)
		{
			AnimationInfo *info = new AnimationInfo();
			info->Name = heapstr(parentAnimationName);
			info->Frames.push_back(mesh);
			animations.push_back(info);
		}
	}
}