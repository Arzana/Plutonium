#pragma once
#include <sal.h>

#if defined(_WIN32) && defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

#if defined(ASSERT)
#undef ASSERT
#endif

namespace Plutonium
{
	/* Defines the basic logging type. */
	enum class LogType
	{
		/* Invalid type, should never be used! */
		None,
		/* Debug only information. */
		Debug,
		/* Verbose information. */
		Info,
		/* Non fatal erro. */
		Warning,
		/* Fatal error. */
		Error
	};

	/* Logs a message to the output without adding a newline character at the end. */
	void _CrtLogNoNewLine(_In_ LogType type, _In_ const char *format, ...);
	/* Logs a message line to the output. */
	void _CrtLog(_In_ LogType type, _In_ const char *format, ...);
	/* Logs a fatal error to the output with a specified sender. */
	void _CrtLogExc(_In_ const char *sender, _In_ const char *file, _In_ const char *func, _In_ int line);
	/* Logs a fatal error to the output and break excecution on debug mode. */
	void _CrtLogThrow(_In_ const char *msg, _In_ const char *file, _In_ const char *func, _In_ int line, _In_ const char *desc, ...);
	/* Sets the render position of the output back a specified amount of places. */
	_Check_return_ bool _CrtLogBacktrack(_In_ size_t amnt);
	/* Waits for the user to press any key in the console. */
	void _CrtPressAnyKeyToContinue(void);
}

#define LOG_MSG(msg, ...)						Plutonium::_CrtLog(Plutonium::LogType::Info, (msg), ##__VA_ARGS__)
#define LOG_MSG_IF(condition, msg, ...)			{ if (condition) LOG_MSG((msg), ##__VA_ARGS__); }

#define LOG_WAR(msg, ...)						Plutonium::_CrtLog(Plutonium::LogType::Warning, (msg), ##__VA_ARGS__)
#define LOG_WAR_IF(condition, msg, ...)			{ if (condition) LOG_WAR((msg), ##__VA_ARGS__); }

#define LOG_THROW(msg, ...)						Plutonium::_CrtLogThrow("An unhandled exception occured!", __FILE__, __FUNCTION__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_THROW_IF(condition, msg, ...)		{ if (condition) LOG_THROW((msg), ##__VA_ARGS__); }

#if defined(DEBUG)
#define LOG(msg, ...)							Plutonium::_CrtLog(Plutonium::LogType::Debug, (msg), ##__VA_ARGS__)
#define LOG_IF(condition, msg, ...)				{ if (condition) LOG((msg), ##__VA_ARGS__); }

#define ASSERT(msg, ...)						LOG_THROW((msg), ##__VA_ARGS__)
#define ASSERT_IF(condition, msg, ...)			LOG_THROW_IF((condition), (msg), ##__VA_ARGS__)
#else
#define LOG(...)
#define LOG_IF(...)

#define ASSERT(...)
#define ASSERT_IF(...)
#endif