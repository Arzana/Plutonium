#include "Graphics\Models\DynamicModel.h"
#include "Graphics\Models\Md2Loader.h"
#include "Streams\FileReader.h"
#include "Core\StringFunctions.h"
#include "Content\AssetLoader.h"
#include <cstring>

using namespace Plutonium;

Plutonium::DynamicModel::DynamicModel(AssetLoader *loader)
	: loader(loader)
{}

Plutonium::DynamicModel::~DynamicModel(void)
{
	free_s(name);
	free_s(path);

	/* Release animation information. */
	for (size_t i = 0; i < animations.size(); i++) delete_s(animations.at(i));
	animations.clear();

	/* Release skin. */
	loader->Unload(skin);
}

void Plutonium::DynamicModel::Finalize(void)
{
	/* Finalize all animation frames. */
	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationInfo *info = animations.at(i);
		for (size_t j = 0; j < info->Frames.size(); j++)
		{
			info->Frames.at(j)->Finalize(loader->GetWindow());
		}
	}
}

DynamicModel * Plutonium::DynamicModel::FromFile(const char * path, AssetLoader *loader, const char * texture)
{
	/* Create result. */
	DynamicModel *result = new DynamicModel(loader);
	result->path = heapstr(path);

	/* Attempt to load raw data. */
	const Md2LoaderResult *raw = _CrtLoadMd2(path);

	/* Load all animation frames. */
	std::vector<Mesh*> meshes;
	for (size_t i = 0; i < raw->frames.size(); i++) meshes.push_back(Mesh::FromFile(raw, i));

	/* Parse frames to animations. */
	result->SplitFrames(meshes);

	/* Construct texture path. */
	FileReader reader(path, true);
	char tex[FILENAME_MAX];
	mrgstr(reader.GetFileDirectory(), texture ? texture : raw->textures.at(0), tex);

	/* Parse texture to result. */
	result->name = heapstr(reader.GetFileNameWithoutExtension());
	result->skin = loader->LoadTexture(tex);
	result->Finalize();	// TODO: Remove!

						/* Log creation. */
	LOG("Finished loading model '%s' (%zu animations).", reader.GetFileName(), result->animations.size());
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