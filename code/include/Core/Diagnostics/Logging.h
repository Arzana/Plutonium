#pragma once
#include <map>
#include <mutex>
#include "Core/String.h"

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

	/* Defines the modes available for fatal messages. */
	enum class RaiseMode
	{
		/* Just logs the fatal exception to the console and continues execution. */
		Ignore,
		/* Creates a crash report file in the specified directory and exits with exit code 1. */
		CrashReport,
		/* Creates a message box to display the error message and alows the used to pick the response. */
		CrashWindow,
		/* Calls a custom function to handle fatal exceptions. */
		Custom
	};

	/* Defines which parts of the log to display. */
	enum class LogDetails
	{
		/* Display no additional information than the message. */
		None = 0,
		/* Displays the timestamp of the log message. */
		Timestamp = 1,
		/* Displays the thread and process ID. */
		Threading = 2,
		/* Displays the log type. */
		Type = 4,
		/* Display a user defined string. */
		UserInfo = 8,
		/* Displays everything except the user info. */
		Default = 7,
		/* Displays all optional log details. */
		All = 15
	};

	/* Defines the custom callback signature used for custom raise callbacks. */
	using RaiseCallback = void(*)(_In_ const char *format, _In_ va_list args);

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

		/* 
		Sets the mode that should be used for fatal messages.
		reportDir is only used if the mode is CrashReport. 
		callback is only used if the mode is Custom.
		*/
		static void SetRaiseMode(_In_ RaiseMode mode, _In_opt_ const wstring &reportDir = nullptr, _In_opt_ RaiseCallback callback = nullptr);
		/* Set the details of the log messages. */
		static void SetDetails(_In_  LogDetails details);
		/* Sets a user defined string that should be logged with every future log entry. */
		static void SetUserInfo(_In_ const string &info);
		/* Makes sure that the output buffer is large enough to fit strings with the specified length. */
		static void SetBufferWidth(_In_ uint32 width);
		/* Moves the output window to a specified location. */
		static void Move(_In_ int32 x, _In_ int32 y);
		/* Resized the output window to a specified size. */
		static void Resize(_In_ uint32 w, _In_ uint32 h);
		/* Attempts to set the render position of the output back a specified amount of places. */
		_Check_return_ static bool BackTrack(_In_ uint32 amount);
		/* Queues a "Press any key to continue..." message to the buffer and waits for the user to press any key. */
		static void PressAnyKeyToContinue(void);

	protected:
		/* Log a message line to the output. */
		void LogMsg(_In_ LogType type, _In_ bool addNl, _In_ const char *format, _In_opt_ va_list args);
		/* Logs a fatal error header to the output with a specified sender (doesn't lock). */
		void LogExcHdr(_In_ const char *sender, _In_ const wstring &file, _In_ const wstring &func, _In_ int32 line);
		/* Logs a fatal error footer to the output (doesn't lock). */
		void LogExcFtr(_In_ uint32 framesToSkip);
		/* Logs a fatal exception to the output and breaks excecution. */
		void LogExc(_In_ const char *msg, _In_ uint32 framesToSkip, _In_opt_ va_list args);

	private:
		bool shouldAddLinePrefix;
		bool suppressLogging;
		LogType lastType;
		const char *typeStr;
		string userInfo;

		RaiseMode mode;
		LogDetails details;
		wstring reportDir;
		RaiseCallback callback;

		std::map<uint64, wstring> processNames;
		std::map<uint64, wstring> threadNames;
		std::mutex printLock;

		Log(void);

		static Log& GetInstance(void);
		static int32 CrtErrorHandler(int32 category, char *msg, int32 *retVal);

		void Raise(const char *msg, va_list args);
		void CreateCrashReport(void);
		void LogMsgVa(LogType type, bool addNl, const char *format, ...);
		void LogMsgInternal(LogType type, bool addNl, size_t len, const char *format, va_list args);
		void UpdateType(LogType type);
		void LogLinePrefix(LogType type);
	};
}