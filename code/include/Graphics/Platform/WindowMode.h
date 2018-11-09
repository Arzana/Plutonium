#pragma once

namespace Pu
{
	/* Defines the modes of a window. */
	enum class WindowMode
	{
		/* A sizeable window with a header. */
		Windowed,
		/* A sizeable window without a header. */
		Borderless,
		/* A sizeable window that takes up the whole display. */
		BorderlessFullscreen,
		/* A fullscreen window. */
		Fullscreen
	};
}