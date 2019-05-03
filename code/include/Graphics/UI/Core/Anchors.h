#pragma once
#include "Core/EnumUtils.h"

namespace Pu
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
		/* The top center of the screen. */
		TopCenter = Top | CenterWidth,
		/* The left center of the screen. */
		LeftCenter = Left | CenterHeight,
		/* The absolute center of the screen. */
		Center = CenterHeight | CenterWidth,
		/* The right center of the screen. */
		RightCenter = Right | CenterHeight,
		/* The bottom left corner of the screen. */
		BottomLeft = Bottom | Left,
		/* The bottom right corner of the screen. */
		BottomRight = Bottom | Right,
		/* The bottom center of the screen. */
		BottomCenter = Bottom | CenterWidth
	};

	/* Adds the two anchors together. */
	_Check_return_ inline Anchors operator |(_In_ Anchors first, _In_ Anchors second)
	{
		return _CrtEnumBitOr(first, second);
	}

	/* Checks whether the input anchor is a correct value. */
	_Check_return_ inline bool IsAnchorValid(_In_ Anchors anchor)
	{
		if (_CrtEnumCheckFlag(anchor, Anchors::Top | Anchors::Bottom)) return false;
		if (_CrtEnumCheckFlag(anchor, Anchors::Left | Anchors::Right)) return false;
		if (_CrtEnumCheckFlag(anchor, Anchors::Left | Anchors::CenterWidth)) return false;
		if (_CrtEnumCheckFlag(anchor, Anchors::Right | Anchors::CenterWidth)) return false;
		if (_CrtEnumCheckFlag(anchor, Anchors::Top | Anchors::CenterHeight)) return false;
		if (_CrtEnumCheckFlag(anchor, Anchors::Bottom | Anchors::CenterHeight)) return false;
		return true;
	}

	/* Checks whether the input anchor is a valid anchor and is not equal to None. */
	_Check_return_ inline bool IsAnchorWorkable(_In_ Anchors anchor)
	{
		return anchor != Anchors::None && IsAnchorValid(anchor);
	}
}