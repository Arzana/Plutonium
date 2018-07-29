#pragma once
#include <sal.h>
#include "Core\Math\Constants.h"
#include "OpenGLSources.h"
#include "OpenGLMessageTypes.h"

struct GLFWwindow;

namespace Plutonium
{
	struct Window;
	/* Gets the number of draw calls since the last reset. */
	_Check_return_ size_t _CrtGetDrawCalls(void);
	/* Resets the number of draw calls to zero. */
	void _CrtResetDrawCalls(void);

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
	/* Adds a message filter to the logging API. */
	void _CrtAddLogRule(_In_ uint32 id, _In_ OpenGLSource api, OpenGLMsgType type, _In_ const char *reason);
}