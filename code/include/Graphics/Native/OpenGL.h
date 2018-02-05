#pragma once
#include <sal.h>

struct GLFWwindow;

namespace Plutonium
{
	struct Window;

	/* Moves the terminal to a move favorable position (the first display that doesn't contain the window.). */
	void _CrtDbgMoveTerminal(_In_ GLFWwindow *gameWindow);
	/* Sets the OS specific setting for synchonization with vertical retrace. */
	void _CrtSetSwapIntervalExt(_In_ int interval);
	/* Attempts to initialize GLFW. */
	_Check_return_ int _CrtInitGLFW(void);
	/* Attempt to initialize Glad. */
	_Check_return_ int _CrtInitGlad(void);
	/* Attempt to finalize GLFW. */
	_Check_return_ void _CrtFinalizeGLFW(void);
}