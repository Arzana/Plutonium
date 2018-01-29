#include "Graphics\Native\OpenGL.h"
#include <glad\glad.h>
#include <glfw3.h>
#include "Core\Math\Rectangle.h"
#include "Core\Logging.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

bool glfwState = false;

void GlfwErrorEventHandler(int code, const char *descr)
{
	LOG_ERR("GLFW has encounterred erro %d (%s)!", code, descr);
}

void GladErrorEventHandler(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar *msg, const void *userParam)
{
	/* Geta human readable error source. */
	const char *caller;
	switch (src)
	{
		case (GL_DEBUG_SOURCE_API):
			caller = "The basic API";
			break;
		case (GL_DEBUG_SOURCE_WINDOW_SYSTEM):
			caller = "The window API";
			break;
		case (GL_DEBUG_SOURCE_SHADER_COMPILER):
			caller = "The shader compiler";
			break;
		case (GL_DEBUG_SOURCE_THIRD_PARTY):
			caller = "A third party";
			break;
		case (GL_DEBUG_SOURCE_APPLICATION):
			caller = "The user";
			break;
		default:
			caller = "An unknown source";
			break;
	}

	/* Get a human readable severity. */
	const char *level;
	switch (severity)
	{
		case (GL_DEBUG_SEVERITY_HIGH):
			level = "a high";
			break;
		case (GL_DEBUG_SEVERITY_MEDIUM):
			level = "a medium";
			break;
		case (GL_DEBUG_SEVERITY_LOW):
			level = "a low";
			break;
		default:
			level = "an insignificant";
			break;
	}

	const char *error;
	switch (type)
	{
		case (GL_DEBUG_TYPE_ERROR):
			error = "basic";
			break;
		case (GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR):
			error = "deprecation";
			break;
		case (GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR):
			error = "undefined behaviour";
			break;
		case (GL_DEBUG_TYPE_PORTABILITY):
			error = "cannot port";
			break;
		case (GL_DEBUG_TYPE_PERFORMANCE):
			error = "performance";
			break;
		default:
			error = "unknown";
			break;
	}

	/* Log human readable error. */
	LOG_ERR("%s caused %s severity %s exception: %s", caller, level, error, msg);
}

void Plutonium::_CrtDbgMoveTerminal(GLFWwindow * gameWindow)
{
	LOG("Attempting to move console to better location.");

	/* Get available displays. */
	int displayCnt = 0;
	GLFWmonitor **displays = glfwGetMonitors(&displayCnt);

	/* Get windows specific terminal handle. */
#if defined(_WIN32)
	HWND hndlr = GetConsoleWindow();
	if (!hndlr)
	{
		LOG_WAR("No console window found!");
		return;
	}
#else
	LOG_WAR("Moving the terminal is not yet supported on this platform!");
	return;
#endif

	/* Get window position attribute. */
	int wx = 0, wy = 0;
	glfwGetWindowPos(gameWindow, &wx, &wy);
	Vector2 gwp = Vector2(static_cast<float>(wx), static_cast<float>(wy));

	/* Get new window position. */
	for (int i = 0; i < displayCnt; i++)
	{
		/* Get monitor bounds. */
		int x = 0, y = 0, w = 0, h = 0;
		glfwGetMonitorPos(displays[i], &x, &y);
		glfwGetMonitorPhysicalSize(displays[i], &w, &h);
		Rectangle bounds = Rectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));

		/* Attempt to move terminal. */
		if (!bounds.Contains(gwp))
		{
#if defined(_WIN32)
			SetWindowPos(hndlr, HWND_TOP, x, 0, 0, 0, SWP_NOSIZE);
			return;
#endif
		}
	}

	LOG_WAR("No improved terminal position found!");
}

int Plutonium::_CrtInitGLFW(void)
{
	/* Make sure we don't activate GLFW multiple times. */
	if (glfwState) return GLFW_TRUE;

	/* Initialize GLFW. */
	if (glfwInit() != GLFW_TRUE)
	{
		LOG_ERR("Failed to initialize GLFW!");
		return GLFW_FALSE;
	}

	/* Set error callback to make sure we log GLFW errors. */
	glfwSetErrorCallback(GlfwErrorEventHandler);

	/* Log results. */
	glfwState = true;
	LOG("Initialized GLFW.");
	return GLFW_TRUE;
}

int Plutonium::_CrtInitGlad(void)
{
	/* Make sure we don't activate Glad multiple times. */
	static bool activatedGlad = false;
	if (activatedGlad) return GLFW_TRUE;

	/* Initialize Glad. */
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
	{
		LOG_ERR("Failed to initialize Glad!");
		return GLFW_FALSE;
	}

	/* Set error callback to make sure we log OpenGL errors. */
	glDebugMessageCallback(GLDEBUGPROC(GladErrorEventHandler), nullptr);

	/* Log results. */
	activatedGlad = true;
	const char *version = (const char*)glGetString(GL_VERSION);
	LOG("Initialized Glad.");
	LOG_MSG("Using OpenGL version: %s.", version);
	return GLFW_TRUE;
}

void Plutonium::_CrtFinalizeGLFW(void)
{
	if (!glfwState) return;
	glfwState = false;
	LOG("Terminating GLFW.");
	glfwTerminate();
}