#pragma once
#include "Core/EnumUtils.h"

namespace Pu
{
	/* Defines the possible options for a dynamic or kinematic object passing a plane. */
	enum class PassOptions
	{
		/* A normal collision respose (first order contraint for dynamic, second order contraint for kinematic). */
		KinematicResponse = 1,
		/* The object is marked to be destroyed soon. */
		Destroy = 2,
		/* A custom event triggers. */
		Event = 4
	};

	/* Adds the two specified pass option flags together. */
	_Check_return_ constexpr inline PassOptions operator |(_In_ PassOptions left, _In_ PassOptions right)
	{
		return _CrtEnumBitOr(left, right);
	}
}