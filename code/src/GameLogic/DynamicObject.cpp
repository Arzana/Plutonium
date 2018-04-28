#include "GameLogic\DynamicObject.h"
#include "Core\Math\Interpolation.h"

Plutonium::DynamicObject::DynamicObject(Game * game, const char * model, const char * texture, int loadWeight, Initializer initializer)
	: parent(game), percentage(loadWeight), model(nullptr), initializer(initializer),
	frameMoveMod(0), accumTime(0.0f), curAnim(0), curFrame(0), nextFrame(0), mixAmnt(0.0f), running(false)
{
	game->GetLoader()->LoadModel(model, Callback<DynamicModel>(this, &DynamicObject::OnLoadComplete), false, texture);
}

Plutonium::DynamicObject::~DynamicObject(void)
{
	parent->GetLoader()->Unload(model);
}

void Plutonium::DynamicObject::PlayAnimation(const char * name)
{
	for (size_t i = 0; i < model->animations.size(); i++)
	{
		/* Attempt to find animation. */
		AnimationInfo *info = model->animations.at(i);
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

void Plutonium::DynamicObject::Update(float dt)
{
	/* Get playing animation. */
	AnimationInfo *info = model->animations.at(curAnim);

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

void Plutonium::DynamicObject::Initialize(void)
{
	/* Initialize all animations. */
	for (size_t i = 0; i < model->animations.size(); i++)
	{
		/* Call initializer. */
		AnimationInfo *info = model->animations.at(i);
		initializer(info->Name, info->Flags, info->TargetFrameTime);

		/* Convert FPS to an elapsed time in seconds. */
		info->TargetFrameTime = recip(info->TargetFrameTime);
	}
}

void Plutonium::DynamicObject::OnLoadComplete(const AssetLoader *, DynamicModel * result)
{
	model = result;
	Initialize();
	parent->UpdateLoadPercentage(percentage);
}

void Plutonium::DynamicObject::MoveFrame(void)
{
	/* Get playing animation. */
	AnimationInfo *info = model->animations.at(curAnim);

	/* Get the flags without the lerping flag. */
	switch (_CrtEnumRemoveFlag(info->Flags, PlayBackFlags::TimeBased))
	{
	case (_CrtInt2Enum<PlayBackFlags>(0)):
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
	case (_CrtEnumBitOr(PlayBackFlags::ToAndFro, PlayBackFlags::Reverse)):
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
	case (_CrtEnumBitOr(PlayBackFlags::Reverse, PlayBackFlags::Loop)):
		/* Decrease frames. */
		if ((curFrame += frameMoveMod) <= 0) curFrame = info->Frames.size() - 1;
		if ((nextFrame += frameMoveMod) <= 0) nextFrame = info->Frames.size() - 1;
		break;
	case (_CrtEnumBitOr(PlayBackFlags::Loop, PlayBackFlags::ToAndFro)):
	case (_CrtEnumBitOr(PlayBackFlags::Reverse, _CrtEnumBitOr(PlayBackFlags::Loop, PlayBackFlags::ToAndFro))):
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