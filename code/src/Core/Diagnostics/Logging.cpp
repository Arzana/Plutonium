#pragma warning(disable:4996)

#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include <conio.h>		// Press ANY key to continue.
#endif

#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Diagnostics/StackTrace.h"
#include "Streams/FileWriter.h"
#include "Core/EnumUtils.h"
#include "Config.h"
#include <cstdio>
#include <ctime>
#include <crtdbg.h>
#include <io.h>
#include <fcntl.h>

using namespace Pu;

void Pu::Log::Verbose(const char * format, ...)
{
	/* Verbose only works on debug so just cast to void on release. */
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(LogType::Debug, true, format, args);
	va_end(args);
#else 
	(void)format;
#endif
}

void Pu::Log::Message(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(LogType::Info, true, format, args);
	va_end(args);
}

void Pu::Log::Warning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(LogType::Warning, true, format, args);
	va_end(args);
}

void Pu::Log::Error(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(LogType::Error, true, format, args);
	va_end(args);
}

void Pu::Log::Fatal(const char * format, ...)
{
	/*
	Skip last three frames:
	- StackFrame::GetStackTrace
	- Log::LogExc
	- Log::Fatal
	*/
	va_list args;
	va_start(args, format);
	GetInstance().LogExc(nullptr, format, 3, args);
	va_end(args);
}

void Pu::Log::Specific(LogType type, const char * format, ...)
{
#ifdef _DEBUG
	if (type == LogType::None) Log::Fatal("LogType cannot be None!");
#else
	if (type == LogType::Debug) return;
#endif

	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(type, true, format, args);
	va_end(args);
}

void Pu::Log::APIFatal(const char * sender, bool condition, const char * format, ...)
{
	if (!condition)
	{
		/*
		Skip last four frames:
		- StackFrame::GetStackTrace
		- Log::LogExcFtr
		- Log::LogExc
		- Log::Fatal
		*/
		va_list args;
		va_start(args, format);
		GetInstance().LogExc(sender, format, 4, args);
		va_end(args);
	}
}

void Pu::Log::SetRaiseMode(RaiseMode mode, const wstring * reportDir, RaiseCallback callback)
{
	GetInstance().mode = mode;

	switch (mode)
	{
	case Pu::RaiseMode::CrashReport:
		if (reportDir)
		{
			GetInstance().reportDir = *reportDir;
			if (reportDir->back() != L'\\') GetInstance().reportDir += L'\\';
		}
		break;
	case Pu::RaiseMode::Custom:
		if (!callback) Log::Error("Raise callback should not be null!");
		GetInstance().callback = callback;
		break;
	}
}

void Pu::Log::SetDetails(LogDetails details)
{
	GetInstance().details = details;
}

void Pu::Log::SetUserInfo(const string & info)
{
	Log &instance = GetInstance();
	instance.userInfo = info;
	instance.details = _CrtEnumAddFlag(instance.details, LogDetails::UserInfo);
}

void Pu::Log::SetBufferWidth(uint32 width)
{
#if defined(_WIN32)
	/* Get terminal handle. */
	const HANDLE terminalHndl = GetStdHandle(STD_OUTPUT_HANDLE);

	/* Get current buffer info. */
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(terminalHndl, &info)) return;

	/* Calculate required width. */
	const SHORT x = info.dwCursorPosition.X + static_cast<SHORT>(width);
	if (x > info.dwSize.X)
	{
		/* Increase terminal buffer size. */
		SetConsoleScreenBufferSize(terminalHndl, { x, info.dwSize.Y });
	}
#else
	Warning("Cannot resize console buffer on this platform!");
#endif
}

void Pu::Log::Move(int32 x, int32 y)
{
#ifdef _WIN32
	/* Get the window handle for the terminal. */
	const HWND terminalHndl = GetConsoleWindow();
	if (!terminalHndl)
	{
		Log::Warning("Could not get Win32 terminal handle!");
		return;
	}

	/* Move the terminal. */
	if (!SetWindowPos(terminalHndl, HWND_TOP, x, y, 0, 0, SWP_NOSIZE))
	{
		Log::Error("Unable to move Win32 terminal to [%d, %d], reason: '%ls'!", x, y, _CrtGetErrorString().c_str());
	}
#else
	Warning("Moving the output window is not supported on this platform!");
#endif
}

void Pu::Log::Resize(uint32 w, uint32 h)
{
#ifdef _WIN32
	/* Get the window handle for the terminal. */
	const HWND terminalHndl = GetConsoleWindow();
	if (!terminalHndl)
	{
		Log::Warning("Could not get Win32 terminal handle!");
		return;
	}

	/* Resize the terminal. */
	if (!SetWindowPos(terminalHndl, HWND_TOP, 0, 0, w, h, SWP_NOMOVE))
	{
		const wstring error = _CrtGetErrorString();
		Log::Error("Unable to resize Win32 terminal to %ux%u, reason: '%ls'!", w, h, error.c_str());
	}
#else
	Warning("Resizing the output window is not supported on this platform!");
#endif
}

bool Pu::Log::BackTrack(uint32 amount)
{
#if defined(_WIN32)
	/* Get console handle. */
	const HANDLE hndlr = GetStdHandle(STD_OUTPUT_HANDLE);
	/* Get current position. */
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hndlr, &csbi)) return false;

	/* Alter position. */
	csbi.dwCursorPosition.X -= static_cast<SHORT>(amount);
	GetInstance().printLock.lock();
	SetConsoleCursorPosition(hndlr, csbi.dwCursorPosition);
	GetInstance().printLock.unlock();
	return true;
#else
	Warning("Backtracking the output is not supported on this platform!");
	return false;
#endif
}

void Pu::Log::PressAnyKeyToContinue(void)
{
#ifdef _WIN32
	Message("Press any key to continue...");
	getch();
#else
	Message("Press the ENTER key to continue...");
	getchar();
#endif
}

void Pu::Log::LogMsg(LogType type, bool addNl, const char * format, va_list args)
{
	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0 && !suppressLogging)
	{
		printLock.lock();
		LogMsgInternal(type, addNl, len, format, args);
		printLock.unlock();
	}
}

void Pu::Log::LogExcHdr(const char * sender, const wstring & file, const wstring & func, int32 line)
{
	constexpr const char *FORMAT = "%s threw an exception!\nFILE:		%ls.\nFUNCTION:	%ls.\nLINE:		%d.\nMESSAGE:	";
	if (!sender) sender = "Undefined caller";
	LogMsgVa(LogType::Error, false, FORMAT, sender, file.c_str(), func.c_str(), line - 1);
}

void Pu::Log::LogExcFtr(uint32 framesToSkip)
{
	/* Get current call stack. */
	vector<StackFrame> callStack;
	StackFrame::GetStackTrace(framesToSkip, callStack);
	string line;

	/* Make sure the color is correct. */
	UpdateType(LogType::Error);

	/*
	Get the information needed to nicely format the stack frames.
	Line 0 doesn't exist so we print unknown for that, which is 7 digits.
	All initial values are the lengths for their headers.
	*/
	size_t maxFunctionNameLength = 8, maxLineLength = 4, maxModuleNameLength = 6, maxFileNameLength = 4;
	for (const StackFrame &cur : callStack)
	{
		if constexpr (!LoggerExternalsVisible)
		{
			if (cur.FunctionName == L"Pu::_CrtPuThreadStart") break;
			if (cur.FunctionName == L"main") break;
		}

		maxFunctionNameLength = max(maxFunctionNameLength, cur.FunctionName.length());
		maxLineLength = max(maxLineLength, cur.Line ? string::count_digits(static_cast<uint64>(cur.Line)) : 7);
		maxModuleNameLength = max(maxModuleNameLength, cur.ModuleName.length());
		maxFileNameLength = max(maxFileNameLength, cur.FileName.length());
	}

	/* Log table header. */
	const string offset1(maxFunctionNameLength / 2, ' ');
	const string offset2((maxFunctionNameLength / 2 - 4) + maxLineLength / 2, ' ');
	const string offset3((maxLineLength / 2 - 2) + maxModuleNameLength / 2, ' ');
	const string offset4((maxModuleNameLength / 2 - 3) + maxFileNameLength / 2, ' ');

	printf("STACKTRACE:\n");
	printf("		%s FUNCTION %s  LINE %s  MODULE %s  FILE\n", offset1.c_str(), offset2.c_str(), offset3.c_str(), offset4.c_str());

	bool suppressLog = false;
	for (const StackFrame &cur : callStack)
	{
		if (!suppressLog)
		{
			/* Print the function name. */
			(line = "		at %-") += std::to_string(maxFunctionNameLength + 1) += "ls";
			printf(line.c_str(), cur.FunctionName.c_str());

			/* Print the line. */
			if (cur.Line)
			{
				(line = "| %-") += std::to_string(maxLineLength + 1) += 'd';
				printf(line.c_str(), cur.Line - 1);
			}
			else printf("| Unknown ");

			/* Print the module name. */
			(line = "| %-") += std::to_string(maxModuleNameLength + 1) += "ls";
			printf(line.c_str(), cur.ModuleName.c_str());

			/* Print the file name. */
			(line = "| %-") += std::to_string(maxFileNameLength + 1) += "ls\n";
			printf(line.c_str(), cur.FileName.c_str());

			/* Stop stacktrace log after either a thread start has been found or main has been found. */
			if constexpr (!LoggerExternalsVisible)
			{
				if (cur.FunctionName == L"Pu::_CrtPuThreadStart") suppressLog = true;
				if (cur.FunctionName == L"main") suppressLog = true;
				if (suppressLog) printf("		[External Code]\n");
			}
		}
	}
}

void Pu::Log::LogExc(const char *sender, const char * msg, uint32 framesToSkip, va_list args)
{
	if (suppressLogging) return;
	suppressLogging = true;

	/* Get last stack frame for file information. */
	StackFrame frame;
	StackFrame::GetCallerInfo(framesToSkip, frame);

	/* 
	Printing an exception is has 3 phases:
	The error header, message and footer. 
	We want no other message to interupt our exception so we handle locking in this function rather than the underlying ones.
	*/
	printLock.lock();

	/* log error header. */
	LogExcHdr(sender, frame.FileName, frame.FunctionName, frame.Line);
	
	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(msg);
	if (len > 0)
	{
		shouldAddLinePrefix = false;
		LogMsgInternal(LogType::Error, true, len, msg, args);
	}

	/* Log error footer. */
	LogExcFtr(framesToSkip + 1);

	/* Make sure that the log cannot be used during a raise event (this is to make sure that the exception is always handled as a exit condition). */
	Raise(msg, args);
	printLock.unlock();
}

Pu::Log::Log(void)
	: shouldAddLinePrefix(true), suppressLogging(false),
	lastType(LogType::None), details(LogDetails::Default),
	typeStr(""), reportDir(L"CrashReports\\"), callback(nullptr),
#ifdef _DEBUG
	mode(RaiseMode::CrashWindow)
#else
	mode(RaiseMode::CrashReport)
#endif
{
#ifdef _WIN32
	_CrtSetReportHook(&CrtErrorHandler);
#endif

	SetBufferWidth(1024);
}

Log & Pu::Log::GetInstance(void)
{
	static Log logger;
	return logger;
}

int32 Pu::Log::CrtErrorHandler(int32 category, char * msg, int32 * retVal)
{
	if (category == _CRT_WARN)
	{
		Log::Warning(msg);
		*retVal = 0;	// Specify that the debugger should not break.
	}
	else
	{
		/*
		Skip last five frames:
		- StackFrame::GetStackTrace
		- Log::LogExcFtr
		- Log::LogExc
		- Log::Fatal
		- CrtErrorHandler
		*/
		GetInstance().LogExc(nullptr, msg, 5, nullptr);
		*retVal = 1;	// Debugger should break (if we don't do that already).
	}

	return true;
}

void Pu::Log::Raise(const char * msg, va_list args)
{
	/* Make sure that we leave the console in the default state before raising. */
	UpdateType(LogType::None);

	switch (mode)
	{
	case RaiseMode::Ignore:
		return;
	case RaiseMode::CrashWindow:
		_CrtDbgBreak();
		break;
	case RaiseMode::CrashReport:
		CreateCrashReport(string::vnprintf(msg, args));
		exit(1);
		break;
	case RaiseMode::Custom:
		if (callback) callback(msg, args);
		else _CrtDbgBreak();
		break;
	}
}

void Pu::Log::CreateCrashReport(const string & msg)
{
	/* We don't want the filewriter to log anything. */
	suppressLogging = true;

	/* Create the crash report file. */
	const time_t now = std::time(nullptr);
	char buffer[100];
	std::strftime(buffer, sizeof(buffer), "%d-%m-%Y_%H-%M-%S", std::localtime(&now));
	FileWriter file(reportDir + L"CrashReport_" + string(buffer).toWide() + L".txt");

	file.Write("Plutonium Crash Report\n");
	file.Write(msg);
	file.Write("\n");

#ifdef _WIN32
	/* Get the handle to the console. */
	const HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
	file.Write("\n\nLog:\n");

	/* Only attempt to get the console lines if we can retrieve the buffer width. */
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo(hconsole, &info))
	{
		/* Create a temporary buffer, lineLen is not used. */
		string line(static_cast<size_t>(info.dwSize.X));
		DWORD lineLen;

		/* Read untill either there are no more lines found or until the function crashes. */
		size_t emptyCount = 0;
		for (SHORT y = 0; ReadConsoleOutputCharacterA(hconsole, line.data(), info.dwSize.X, { 0, y }, &lineLen); y++)
		{
			/* Remove trailing whitespaces. */
			const string trimmed = line.trim_back(" \t");
			if (trimmed.empty())
			{
				/* We allow for up to 3 whitespaces before canceling the log. */
				if (++emptyCount > 3) break;
			}
			else
			{
				/* Add the logged line to the file's content. */
				file.Write(reinterpret_cast<const byte*>(trimmed.c_str()), 0, trimmed.size());
				file.Write('\n');
				emptyCount = 0;
			}
		}
	}
#endif

	/* Append the loaded modules to the crash report. */
	file.Write("\n\nLoaded Modules:\n");
	for (const wstring &cur : _CrtGetLoadedModules(_CrtGetCurrentProcessId()))
	{
		file.Write(cur.toUTF8() + '\n');
	}
}

void Pu::Log::LogMsgVa(LogType type, bool addNl, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	LogMsgInternal(type, addNl, strlen(format), format, args);
	va_end(args);
}

void Pu::Log::LogMsgInternal(LogType type, bool addNl, size_t len, const char * format, va_list args)
{
	/* Check if new line needs to be added. */
	if (shouldAddLinePrefix) LogLinePrefix(type);
	shouldAddLinePrefix = true;

	/* Log to output and add newline if needed. */
	vprintf(format, args);
	if (format[len - 1] != '\n' && addNl) printf("\n");
}

void Pu::Log::UpdateType(LogType type)
{
	lastType = type;

#ifdef _WIN32
	uint16 typeClr;
#endif

	switch (type)
	{
	case (LogType::Debug):
		typeStr = "Debug";
#ifdef _WIN32
		typeClr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#endif
		break;
	case (LogType::Info):
		typeStr = "Info";
#ifdef _WIN32
		typeClr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#endif
		break;
	case (LogType::Warning):
		typeStr = "Warning";
#ifdef _WIN32
		typeClr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
#endif
		break;
	case (LogType::Error):
		typeStr = "Error";
#ifdef _WIN32
		typeClr = FOREGROUND_RED;
#endif
		break;
	default:
		typeStr = "NULL";
#ifdef _WIN32
		/* This case is used just after raising an exception so we return back to the Windows default. */
		typeClr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#endif
		break;
	}

#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), typeClr);
#endif
}

void Pu::Log::LogLinePrefix(LogType type)
{
	/* Update type prefix. */
	bool loggedPrefix = false;
	if (type != lastType) UpdateType(type);

	if (_CrtEnumCheckFlag(details, LogDetails::Timestamp))
	{
		/* Gets the current milliseconds. */
		constexpr float nano2milli = 1.0f / 10000000.0f;
		timespec ts;
		timespec_get(&ts, TIME_UTC);
		int32 millisec = static_cast<int32>(ts.tv_nsec * nano2milli);

		/* Attempt to get the current time. */
		const time_t now = std::time(nullptr);
		char buffer[100];
		if (std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&now)) == 0) return;

		printf("[%s:%02d]", buffer, millisec);
		loggedPrefix = true;
	}

	if (_CrtEnumCheckFlag(details, LogDetails::Threading))
	{
		/* If any of the funtions in here start to log we end up in an endless loop. */
		suppressLogging = true;

		/* Get process id. */
		const uint64 pid = _CrtGetCurrentProcessId();
		if (processNames.find(pid) == processNames.end()) processNames.emplace(pid, _CrtGetProcessNameFromId(pid));

		/* Get thread id. */
		const uint64 tid = _CrtGetCurrentThreadId();
		if (threadNames.find(tid) == threadNames.end()) threadNames.emplace(tid, _CrtGetThreadNameFromId(tid));
		suppressLogging = false;

		printf("[%ls/%ls]", processNames[pid].c_str(), threadNames[tid].c_str());
		loggedPrefix = true;
	}

	if (_CrtEnumCheckFlag(details, LogDetails::Type))
	{
		printf("[%s]", typeStr);
		loggedPrefix = true;
	}

	if (_CrtEnumCheckFlag(details, LogDetails::UserInfo))
	{
		printf("[%s]", userInfo.c_str());
		loggedPrefix = true;
	}

	/* Add a litte seperator if we logged any prefix. */
	if (loggedPrefix) printf(": ");
}