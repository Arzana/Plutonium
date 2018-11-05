#pragma once
#include <map>
#include <mutex>
#include "Core/String.h"
#include "Core/Math/Constants.h"

#if defined (ASSERT)
#undef ASSERT
#endif

namespace Pu
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

	/* Defines an application global interface for the logging pipeline. */
	class Log
	{
	public:
		Log(_In_ const Log &) = delete;
		Log(_In_ Log &&) = delete;

		_Check_return_ Log& operator =(_In_ const Log &) = delete;
		_Check_return_ Log& operator =(_In_ Log &&) = delete;

		/* Logs a verbose message to the output (Debug only!). */
		static void Verbose(_In_ const char *format, _In_opt_ ...);
		/* Logs a information message to the output. */
		static void Message(_In_ const char *format, _In_opt_ ...);
		/* Logs a warning message to the output. */
		static void Warning(_In_ const char *format, _In_opt_ ...);
		/* Logs an error message to the output (Doesn't raise an std::exception!). */
		static void Error(_In_ const char *format, _In_opt_ ...);
		/* Logs a fatal error message to the output and raises a std::exception. */
		static void Fatal(_In_ const char *format, _In_opt_ ...);

		/* Makes sure that the output buffer is large enough to fit strings with the specified length. */
		static void ResizeIfNeeded(_In_ uint32 width);
		/* Attempts to set the render position of the output back a specified amount of places. */
		_Check_return_ static bool BackTrack(_In_ uint32 amount);
		/* Queues a "Press any key to continue..." message to the buffer and waits for the user to press any key. */
		static void PressAnyKeyToContinue(void);

	protected:
		/* Log a message line to the output. */
		void LogMsg(_In_ LogType type, _In_ bool addNl, _In_ const char *format, _In_opt_ va_list args);
		/* Logs a fatal error header to the output with a specified sender. */
		void LogExcHdr(_In_ const char *sender, _In_ string file, _In_ string func, _In_ int32 line);
		/* Logs a fatal error footer to the output. */
		void LogExcFtr(_In_ uint32 framesToSkip);
		/* Logs a fatal exception to the output and breaks excecution. */
		void LogExc(_In_ const char *msg, _In_ const char *format, _In_opt_ va_list args);

	private:
		bool shouldAddLinePrefix;
		bool suppressLogging;
		LogType lastType;
		const char *typeStr;

		std::map<uint64, string> processNames;
		std::map<uint64, string> threadNames;

		std::mutex printLock;

		Log(void);

		static Log& GetInstance(void);

		void LogMsgVa(LogType type, bool addNl, const char *format, _In_opt_ ...);
		void UpdateType(LogType type);
		void LogLinePrefix(LogType type);
	};
}