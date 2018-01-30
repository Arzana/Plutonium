#include "Graphics\Native\OpenGL.h"
#include "Graphics\Native\Monitor.h"
#include "Core\Logging.h"
#include <glad\glad.h>
#include <glfw3.h>

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
	LOG("Attempting to move terminal to better location.");

	/* Get window position attribute. */
	int x = 0, y = 0;
	glfwGetWindowPos(gameWindow, &x, &y);
	Vector2 pos = Vector2(static_cast<float>(x), static_cast<float>(y));

	/* Get available displays. */
	std::vector<MonitorInfo> displays = MonitorInfo::GetAll();
	for (size_t i = 0; i < displays.size(); i++)
	{
		MonitorInfo cur = displays.at(i);

		/* Check if monitor doesn't contain window. */
		if (!cur.GetWindowBounds().Contains(pos))
		{
#if defined(_WIN32)
			/* Get windows specific terminal handle. */
			HWND terminalHndlr = GetConsoleWindow();
			if (!terminalHndlr)
			{
				LOG_WAR("Could not get windows terminal handler!");
				return;
			}

			/* Move terminal on windows. */
			SetWindowPos(terminalHndlr, HWND_TOP, cur.X, 0, 0, 0, SWP_NOSIZE);
			LOG("Moved terminal to '%s'.", cur.Name);
#else
			LOG_WAR("Moving the terminal is not yet supported on this platform!");
#endif
			return;
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