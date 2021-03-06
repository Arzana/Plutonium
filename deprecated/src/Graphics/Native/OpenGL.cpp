#include "Graphics\Native\OpenGL.h"
#include "Graphics\Native\Monitor.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\Diagnostics\Logging.h"
#include "Core\StringFunctions.h"
#include "Core\EnumUtils.h"
#include <glfw3.h>

/*
Forces the use of the NVidia graphics card instead on onboard GPU.
http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
*/
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

using namespace Plutonium;

/* Whether GLFW has been initialized yet. */
bool glfwState = false;

void GlfwErrorEventHandler(int code, const char *descr)
{
	/* Get human readable exception. */
	const char *error;
	switch (code)
	{
	case (GLFW_NOT_INITIALIZED):
		error = "a not initialized";
		break;
	case (GLFW_NO_CURRENT_CONTEXT):
		error = "a no current context";
		break;
	case (GLFW_INVALID_ENUM):
		error = "a invalid enum";
		break;
	case (GLFW_INVALID_VALUE):
		error = "a invalid value";
		break;
	case (GLFW_OUT_OF_MEMORY):
		error = "a out of memory";
		break;
	case (GLFW_API_UNAVAILABLE):
		error = "a API unavailable";
		break;
	case (GLFW_VERSION_UNAVAILABLE):
		error = "a version unavailable";
		break;
	case (GLFW_PLATFORM_ERROR):
		error = "a platform";
		break;
	case (GLFW_FORMAT_UNAVAILABLE):
		error = "a format unavailable";
		break;
	default:
		error = "an unknown";
		break;
	}

	/*
	Get the caller information if possible.
	We skip the first four frames, these are:
	- _CrtGetCallerInfo							(Because this is just to get the caller information.)
	- GlfwErrorEventHandler						(Because this is the handler for a GLFW exception in this framework.)
	- _glfwInputError							(Because this is GLFW's handler for a exception.)
	- <Whatever function caused the exception>	(Because we can't get the file information from this.)
	*/
	const StackFrame *frame = _CrtGetCallerInfo(4);

	/* Throw exception. */
	_CrtLogExc("GLFW", frame->FileName, frame->FunctionName, frame->Line);
	_CrtLog(LogType::Error, "Encountered %s exception (%d)!\nDESCRIPTION:	%s.", error, code, descr);
	_CrtLogExc(4);

	delete frame;
	throw;
}

#if defined(DEBUG)
uint64 lastGladFramePtr = 0, drawCalls = 0;
const char *lastGladFuncName = "";

void GladPreGLCallEventHandler(const char *name, void*, int, ...)
{
	lastGladFramePtr = Plutonium::_CrtGetCallerPtr(3);
	lastGladFuncName = name;

	if (cntstr(name, "glDraw")) ++drawCalls;
}
#endif

void GladErrorEventHandler(GLenum src, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *msg, const void*)
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
	LogType howToLog;
	switch (severity)
	{
	case (GL_DEBUG_SEVERITY_HIGH):
		level = "a high";
		howToLog = LogType::Error;
		break;
	case (GL_DEBUG_SEVERITY_MEDIUM):
		level = "a medium";
		howToLog = LogType::Warning;
		break;
	case (GL_DEBUG_SEVERITY_LOW):
		level = "a low";
		howToLog = LogType::Warning;
		break;
	default:
		level = "an insignificant";
		howToLog = LogType::Info;
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

	/* Log exception. */
	if (howToLog != LogType::Error) _CrtLog(howToLog, msg);
	else
	{
#if defined(DEBUG)
		/*
		Because the OpenGl calls are handled by the graphics device driver we must get the last stackframe from glad itself.
		On debug mode we save the last stack frame before every glad call. This is very performance intensive but will help with debugging.
		This excludes the top 3 stackframes these are:
		- _CrtGetCallerPtr							(Because this is just to get the caller information.)
		- GladPreGLCallEventHandler					(Because this is the handler for a glad exception in this framework.)
		- <Whatever function caused the exception>	(Because we can't get the file information from this.)
		*/
		const StackFrame *frame = _CrtGetCallerInfoFromPtr(lastGladFramePtr);

		LOG_WAR("The file and function information that is displayed is for the last OpenGL call; not necessarily the one causing the exception!");
		_CrtLogExc("OpenGL", frame->FileName, frame->FunctionName, frame->Line);
		_CrtLog(LogType::Error, "%s(%s) caused %s severity %s exception!\nDESCRIPTION:	%s", caller, lastGladFuncName, level, error, msg);

		delete frame;
		throw;
#else
		_CrtLogExc("OpenGL", "UNKNOWN", "UNKNOWN", 0);
		_CrtLog(LogType::Error, "%s caused %s severity %s exception!\nDESCRIPTION:	%s", caller, level, error, msg);
		throw;
#endif
	}
}

#if defined(DEBUG)
void GladPostGLCallEventHandler(const char *name, void *, int, ...)
{
	GLenum code = glad_glGetError();
	if (code != GL_NO_ERROR) GladErrorEventHandler(0, code, 0, 0, 0, name, nullptr);
}
#endif

size_t Plutonium::_CrtGetDrawCalls(void)
{
#if defined (DEBUG)
	return drawCalls;
#else
	LOG_WAR_ONCE("Can only request number of draw calls on debug mode!");
	return 0;
#endif
}

void Plutonium::_CrtResetDrawCalls(void)
{
#if defined (DEBUG)
	drawCalls = 0;
#endif
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

			/* Get current terminal size. */
			RECT bounds;
			GetWindowRect(terminalHndlr, &bounds);

			/* Move and resize terminal. */
			SetWindowPos(terminalHndlr, HWND_TOP, cur.X, 0, 0, 0, SWP_NOSIZE);
			SetWindowPos(terminalHndlr, HWND_TOP, 0, 0, cur.ClientWidth, bounds.bottom, SWP_NOMOVE);
			LOG("Moved terminal to '%s' and resized it to (%dx%d).", cur.Name, cur.ClientWidth, bounds.bottom);
#else
			LOG_WAR("Moving the terminal is not yet supported on this platform!");
#endif
			/* Resize the console's buffer width. */
			_CrtResizeConsoleIfNeeded(1024);
			return;
		}
	}

	LOG_WAR("No improved terminal position found!");
}

/* Define extension function needed. */
typedef void (APIENTRYP PfnWglSwapIntervalExtProc)(int interval);
PfnWglSwapIntervalExtProc pu_wglSwapIntervalEXT;
#define wglSwapIntervalEXT pu_impl_wglSwapIntervalEXT

void APIENTRY pu_impl_wglSwapIntervalEXT(int interval)
{
#if defined(DEBUG)
	GladPreGLCallEventHandler("wglSwapIntervalEXT", (void*)wglSwapIntervalEXT, 1, interval);
#endif
	pu_wglSwapIntervalEXT(interval);
#if defined(DEBUG)
	GladPostGLCallEventHandler("wglSwapIntervalEXT", (void*)wglSwapIntervalEXT, 1, interval);
#endif
}

void Plutonium::_CrtSetSwapIntervalExt(int interval)
{
	if (_CrtExtensionSupported("WGL_EXT_swap_control"))
	{
		wglSwapIntervalEXT(interval);
	}
	else LOG_WAR_ONCE("Swap control is not supported on this platform!");
}

int Plutonium::_CrtInitGLFW(void)
{
	/* Make sure we don't activate GLFW multiple times. */
	if (glfwState) return GLFW_TRUE;

	/* Initialize GLFW. */
	if (glfwInit() != GLFW_TRUE)
	{
		LOG_THROW("Failed to initialize GLFW!");
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
		LOG_THROW("Failed to initialize Glad!");
	}

#if defined(DEBUG)
	/* Set error callback to make sure we log OpenGL errors. */
	LOG_WAR("Debug mode is enabled! This has a significant performance cost, switch to release mode for optimization.");
	glEnable(GL_DEBUG_OUTPUT);
	glad_set_pre_callback(GLADcallback(GladPreGLCallEventHandler));
	glad_set_post_callback(GLADcallback(GladPostGLCallEventHandler));
	glDebugMessageCallback(GLDEBUGPROC(GladErrorEventHandler), nullptr);

	/* Add specific filters to the message API. */
	_CrtAddLogRule(131185, OpenGLSource::Basic, OpenGLMsgType::Other, "Plutonium handles buffer creation messages");				// NVidea buffer creation messages.
	_CrtAddLogRule(131154, OpenGLSource::Basic, OpenGLMsgType::Performance, "Video capture software is a known performance issue");	// Framebuffer captures used by video software.
#endif

	/* Log results. */
	activatedGlad = true;

	const DeviceInfo *device = _CrtGetDeviceInfo();
	LOG("Initialized Glad.");
	LOG_MSG("Using OpenGL driver %s(%s).", device->DriverVersion, device->DeviceConfig);

	delete device;
	return GLFW_TRUE;
}

void Plutonium::_CrtFinalizeGLFW(void)
{
	if (!glfwState) return;
	glfwState = false;
	LOG("Terminating GLFW.");
	glfwTerminate();
}

#if defined(DEBUG)
void Plutonium::_CrtAddLogRule(uint32 id, OpenGLSource api, OpenGLMsgType type, const char * reason)
#else
void Plutonium::_CrtAddLogRule(uint32 id, OpenGLSource api, OpenGLMsgType type, const char*)
#endif
{
	glDebugMessageControl(_CrtEnum2Int(api), _CrtEnum2Int(type), GL_DONT_CARE, 1, &id, GL_FALSE);
	LOG("Ingoring OpenGL message %u, reason: %s!", id, reason);
}

bool Plutonium::_CrtExtensionSupported(const char * extension)
{
	/* Get the amount of extensions on this system. */
	GLint extensionCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

	for (GLint i = 0; i < extensionCount; i++)
	{
		/* Check if the device supports the needed extension. */
		const char *cur = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (eqlstr(cur, extension)) return true;
	}

	return false;
}