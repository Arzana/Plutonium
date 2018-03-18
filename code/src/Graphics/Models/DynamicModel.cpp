#include "Graphics\Models\DynamicModel.h"
#include "Graphics\Models\Md2Loader.h"
#include "Streams\FileReader.h"
#include "Core\StringFunctions.h"
#include "Core\Math\Interpolation.h"
#include <cstring>

#define enum2int(x)			static_cast<int32>(x)
#define int2enum(t, x)		static_cast<t>(x)
#define removelerp(x)		int2enum(PlayBackFlags, enum2int(x) & ~enum2int(PlayBackFlags::TimeBased))
#define enumor(t, x, y)		int2enum(t, (enum2int(x) | enum2int(y)))
#define enumor2(t, x, y, z) int2enum(t, (enum2int(x) | enum2int(y) | enum2int(z)))

using namespace Plutonium;

Plutonium::DynamicModel::DynamicModel(void)
	: WorldObject(), frameMoveMod(0), accumTime(0.0f),
	curAnim(0), curFrame(0), nextFrame(0), mixAmnt(0.0f), running(false)
{}

Plutonium::DynamicModel::~DynamicModel(void)
{
	/* Release animation information. */
	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationInfo *cur = animations.at(i);
		delete_s(cur);
	}

	animations.clear();

	/* Release skin. */
	delete_s(skin);
}

DynamicModel * Plutonium::DynamicModel::FromFile(const char * path, const char * texture)
{
	/* Create result. */
	DynamicModel *result = new DynamicModel();

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
	result->skin = Texture::FromFile(tex);
	result->Finalize();	// TODO: Remove!

	/* Log creation. */
	LOG("Finished loading model '%s' (%zu animations).", reader.GetFileName(), result->animations.size());
	return result;
}

void Plutonium::DynamicModel::PlayAnimation(const char * name)
{
	for (size_t i = 0; i < animations.size(); i++)
	{
		/* Attempt to find animation. */
		AnimationInfo *info = animations.at(i);
		if (!strcmp(info->Name, name))
		{
			/* Set specific animation. */
			curAnim = i;
			running = true;
			accumTime = 0.0f;

			/* Set rendering and update parameters. */
			frameMoveMod = _CrtGetPlaybackStep(info->Flags);
			curFrame = frameMoveMod > 0 ? 0 : info->Frames.size() - 2;
			nextFrame = curFrame + 1;
			return;
		}
	}

	ASSERT("Attempting to play unknown animation '%s'!", name);
}

void Plutonium::DynamicModel::Initialize(Initializer func)
{
	/* Initialize all animations. */
	for (size_t i = 0; i < animations.size(); i++)
	{
		/* Call initializer. */
		AnimationInfo *info = animations.at(i);
		func(info->Name, info->Flags, info->TargetFrameTime);

		/* Convert FPS to an elapsed time in seconds. */
		info->TargetFrameTime = recip(info->TargetFrameTime);
	}
}

void Plutonium::DynamicModel::Update(float dt)
{
	/* Get playing animation. */
	AnimationInfo *info = animations.at(curAnim);

	/* Update timer and if it's above the threshold update the frame. */
	if (running && (accumTime += dt) > info->TargetFrameTime)
	{
		/* Reset time. */
		accumTime = 0.0f;

		/* Update accordingly. */
		MoveFrame();
	}

	/* Update the mix amount. */
	if (_CrtGetPlaybackShouldInterpolate(info->Flags))
	{
		/* If the animation is has the ToAndFro flag we need to inverse the inverse lerp value. */
		float value = _CrtGetPlaybackShouldAlter(info->Flags) && frameMoveMod < 0 ? info->TargetFrameTime - accumTime : accumTime;
		mixAmnt = ilerp(0.0f, info->TargetFrameTime, value);
	}
	else mixAmnt = 0.0f;
}

void Plutonium::DynamicModel::MoveFrame(void)
{
	/* Get playing animation. */
	AnimationInfo *info = animations.at(curAnim);

	/* Get the flags without the lerping flag. */
	switch (removelerp(info->Flags))
	{
	case (int2enum(PlayBackFlags, 0)):
		/* Increase next frame untill end is reached. */
		if ((nextFrame += frameMoveMod) >= info->Frames.size())
		{
			--nextFrame;
			running = false;
		}
		else ++curFrame;
		break;
	case (PlayBackFlags::Reverse):
		/* Decrease next frame untill end is reached. */
		if ((nextFrame += frameMoveMod) <= 0)
		{
			++nextFrame;
			running = false;
		}
		else --curFrame;
		break;
	case (PlayBackFlags::ToAndFro):
		/* Apply frame movement. */
		nextFrame += frameMoveMod;

		/* Check for end, if reached; swap frame movement. */
		if (nextFrame >= info->Frames.size())
		{
			frameMoveMod = -1;
			--nextFrame;
		}
		/* Check for start, if reached; stop movement. */
		else if (nextFrame <= 0)
		{
			++nextFrame;
			running = false;
		}
		else curFrame += frameMoveMod;
		break;
	case (enumor(PlayBackFlags, PlayBackFlags::ToAndFro, PlayBackFlags::Reverse)):
		/* Apply frame movement. */
		nextFrame += frameMoveMod;

		/* Check for start, if reached; swap frame movement. */
		if (nextFrame <= 0)
		{
			frameMoveMod = 1;
			++nextFrame;
		}
		/* Check for end, if reached; stop movement. */
		else if (nextFrame >= info->Frames.size() - 1)
		{
			--nextFrame;
			running = false;
		}
		else curFrame += frameMoveMod;
		break;
	case (PlayBackFlags::Loop):
		/* Increase frames. */
		curFrame = (curFrame + frameMoveMod) % info->Frames.size();
		nextFrame = (nextFrame + frameMoveMod) % info->Frames.size();
		break;
	case (enumor(PlayBackFlags, PlayBackFlags::Reverse, PlayBackFlags::Loop)):
		/* Decrease frames. */
		if ((curFrame += frameMoveMod) <= 0) curFrame = info->Frames.size() - 1;
		if ((nextFrame += frameMoveMod) <= 0) nextFrame = info->Frames.size() - 1;
		break;
	case (enumor(PlayBackFlags, PlayBackFlags::Loop, PlayBackFlags::ToAndFro)):
	case (enumor2(PlayBackFlags, PlayBackFlags::Reverse, PlayBackFlags::Loop, PlayBackFlags::ToAndFro)):
		/* Apply frame movement. */
		nextFrame += frameMoveMod;

		/* If end is reached, reverse movement. */
		if (nextFrame >= info->Frames.size())
		{
			frameMoveMod = -1;
			--nextFrame;
		}
		/* If start is reached, reverse movement. */
		else if (nextFrame <= 0)
		{
			frameMoveMod = 1;
			++nextFrame;
		}
		else curFrame += frameMoveMod;
		break;
	}
}

void Plutonium::DynamicModel::Finalize(void)
{
	/* Finalize all animation frames. */
	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationInfo *info = animations.at(i);
		for (size_t j = 0; j < info->Frames.size(); j++)
		{
			info->Frames.at(j)->Finalize();
		}
	}
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