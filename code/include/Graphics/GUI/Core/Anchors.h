#pragma once
#include "Core\EnumUtils.h"

namespace Plutonium
{
	/* Defines the relative position a GuiItem should take. */
	enum class Anchors
	{
		/* Default value, no anchor position. */
		None = 0,
		/* Center of the screen on the horizontal axis. */
		CenterWidth = 1,
		/* Center of the screen on vertical axis. */
		CenterHeight = 2,
		/* The left side of the screen. */
		Left = 4,
		/* The right side of the screen. */
		Right = 8,
		/* The top side of the screen. */
		Top = 16,
		/* The bottom side of the screen. */
		Bottom = 32,
		/* The top left corner of the screen. */
		TopLeft = Top | Left,
		/* The top right corner of the screen. */
		TopRight = Top | Right,
		/* The absolute center of the screen. */
		Center = CenterHeight | CenterWidth,
		/* The bottom left corner of the screen. */
		BottomLeft = Bottom | Left,
		/* The bottom right corner of the screen. */
		BottomRight = Bottom | Right
	};

	/* Checks whether the input value is a correct anchor. */
	_Check_return_ inline bool _CrtIsAnchorValid(_In_ Anchors anchor)
	{
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Top, Anchors::Bottom))) return false;
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Left, Anchors::Right))) return false;
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Left, Anchors::CenterWidth))) return false;
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Right, Anchors::CenterWidth))) return false;
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Top, Anchors::CenterHeight))) return false;
		if (_CrtEnumCheckFlag(anchor, _CrtEnumBitOr(Anchors::Bottom, Anchors::CenterHeight))) return false;
		return true;
	}

	/* Checks whether the input value is a valid type and is not equal to None. */
	_Check_return_ inline bool _CrtIsAnchorWorkable(_In_ Anchors anchor)
	{
		return anchor != Anchors::None && _CrtIsAnchorValid(anchor);
	}
}