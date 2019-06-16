#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines some common sliders on input devices. */
	enum class Sliders
		: uint16
	{
#pragma region XBOX
		XBoxLeftThumbY = 0,
		XBoxLeftThumbX = 1,
		XBoxRightThumbY = 2,
		XBoxRightThumbX = 3
#pragma endregion
	};
}