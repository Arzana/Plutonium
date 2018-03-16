#pragma once
#include <sal.h>
#include "Graphics\Models\Mesh.h"

namespace Plutonium
{
	/* Defines rendring flags for animations. */
	enum class PlayBackFlags
	{
		/* Defines an invalid playback frag type. */
		Empty = -1,
		/* The animations should play in reverse order. */
		Reverse = 0b0001,
		/* The animation should alter it's playback order once the end is reached. */
		ToAndFro = 0b0010,
		/* The animation should loop. */
		Loop = 0b0100,
		/* There should be time based interpolation between frames. */
		TimeBased = 0b1000,

		/* Defines the default Playback flags. */
		Default = TimeBased,
		/* Defines the default looping Playback flags. */
		DefaultLoop = Loop | Default
	};

	/* Gets the frame movement for once a frame change event occurs (1 for normal; -1 for reverse). */
	_Check_return_ inline int32 _CrtGetPlaybackStep(_In_ PlayBackFlags flags)
	{
		return (~((static_cast<int32>(flags) & static_cast<int32>(PlayBackFlags::Reverse)) +
			(static_cast<int32>(flags) & static_cast<int32>(PlayBackFlags::Reverse)))) + 2;
	}

	/* Gets whether the playback flags contain the ToAndFro flag. */
	_Check_return_ inline bool _CrtGetPlaybackShouldAlter(_In_ PlayBackFlags flags)
	{
		return (static_cast<int32>(flags) & static_cast<int32>(PlayBackFlags::ToAndFro)) != 0;
	}

	/* Gets whether the playback flags contain the TimeBases flag. */
	_Check_return_ inline bool _CrtGetPlaybackShouldInterpolate(_In_ PlayBackFlags flags)
	{
		return (static_cast<int32>(flags) & static_cast<int32>(PlayBackFlags::TimeBased)) != 0;
	}

	/* Defines per animation information. */
	struct AnimationInfo
	{
		/* The name of the animation. */
		const char *Name;
		/* The amount of time each frame should be displayed for. */
		float TargetFrameTime;
		/* The rendering flags of the animation. */
		PlayBackFlags Flags;
		/* The individual frames of the animation. */
		std::vector<Mesh*> Frames;

		/* Initializes a new instance of a animation info structure. */
		AnimationInfo(void)
			: Name(nullptr), TargetFrameTime(0.0f), Flags(PlayBackFlags::Empty)
		{}

		AnimationInfo(_In_ const AnimationInfo &value) = delete;
		AnimationInfo(_In_ AnimationInfo &&value) = delete;

		/* Releases the resources allocated by the animation. */
		~AnimationInfo(void)
		{
			free_cstr_s(Name);

			for (size_t i = 0; i < Frames.size(); i++)
			{
				Mesh *frame = Frames.at(i);
				delete_s(frame);
			}
			Frames.clear();
		}

		_Check_return_ AnimationInfo& operator =(_In_ const AnimationInfo &other) = delete;
		_Check_return_ AnimationInfo& operator =(_In_ AnimationInfo &&other) = delete;
	};
}