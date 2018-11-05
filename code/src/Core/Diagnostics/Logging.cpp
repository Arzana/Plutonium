#pragma warning(disable:4996)

#include "Core/Diagnostics/StackTrace.h"
#include "Core/Threading/ThreadUtils.h"
#include "Core/SafeMemory.h"
#include <cstdio>
#include <ctime>
#include <crtdbg.h>

#ifdef _WIN32
#include <Windows.h>	// Console colors.
#include <conio.h>		// Press ANY key to continue.
#endif

using namespace Pu;

void Pu::Log::Verbose(const char * format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	GetInstance().LogMsg(LogType::Debug, true, format, args);
	va_end(args);
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
	va_list args;
	va_start(args, format);
	GetInstance().LogExc("An unhandled exception occurred!", format, args);
	va_end(args);
}

void Pu::Log::ResizeIfNeeded(uint32 width)
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
		/* Lock logger. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) LogLinePrefix(type);
		shouldAddLinePrefix = true;

		/* Log to output and add newline if needed. */
		vprintf(format, args);
		if (format[len - 1] != '\n' && addNl) printf("\n");

		/* Unlock logger. */
		printLock.unlock();
	}
}

void Pu::Log::LogExcHdr(const char * sender, string file, string func, int32 line)
{
	constexpr const char *FORMAT = "%s threw an exception!\nFILE:		%s.\nFUNCTION:	%s.\nLINE:		%d.\nMESSAGE:	";
	if (!sender) sender = "Undefined caller";
	LogMsgVa(LogType::Error, false, FORMAT, sender, file.c_str(), func.c_str(), line - 1);
}

void Pu::Log::LogExcFtr(uint32 framesToSkip)
{
	/* Get current call stack. */
	vector<StackFrame> callStack;
	StackFrame::GetStackTrace(framesToSkip, callStack);

	/* Lock logger. */
	printLock.lock();

	/* Make sure the color is correct. */
	UpdateType(LogType::Error);

	/* Log table header. */
	printf("STACKTRACE:\n");
	printf("		%46s FUNCTION %54s LINE %8s MODULE %16s FILE\n", "", "", "", "");

	bool suppressLog = false;
	for (size_t i = 0; i < callStack.size(); i++)
	{
		const StackFrame &cur = callStack[i];

		if (!suppressLog)
		{
			/* Log stack trace, stackframe always advances by one so subtract one. */
			printf("		at %-102s ", cur.FunctionName.c_str());
			printf(cur.Line ? "| %-10d " : "| Unknown    ", cur.Line - 1);
			printf("| %-16s", cur.ModuleName.c_str());
			printf("| %-64s\n", cur.FileName.c_str());

			/* Stop stacktrace log after either a thread start has been found or main has been found. */
			if (strstr(cur.FunctionName.c_str(), "_CrtPuThreadStart")) suppressLog = true;
			if (!strcmp(cur.FunctionName.c_str(), "main")) suppressLog = true;
			if (suppressLog) printf("		[External Code]\n");
		}
	}

	/* Unlock logger. */
	printLock.unlock();
}

void Pu::Log::LogExc(const char * msg, const char * format, va_list args)
{
	/* Get last stack frame for file information. */
	StackFrame frame;
	StackFrame::GetCallerInfo(3, frame);

	/* log error header. */
	LogExcHdr(nullptr, frame.FileName, frame.FunctionName, frame.Line);

	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0 && !suppressLogging)
	{
		/* Lock logger. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) LogLinePrefix(LogType::Error);
		shouldAddLinePrefix = true;

		/* Log to output and add newline if needed. */
		vprintf(format, args);
		if (format[len - 1] != '\n') printf("\n");

		/* Unlock logger. */
		printLock.unlock();
	}

	/*
	Log error footer.
	Skip last four frames:
	- StackFrame::GetStackTrace
	- Log::LogExcFtr
	- Log::LogExc
	- Log::Fatal
	*/
	LogExcFtr(4);

	/* Make sure we halt excecution for all threads. */
	throw std::exception(msg);
}

Pu::Log::Log(void)
	: shouldAddLinePrefix(true), suppressLogging(false),
	lastType(LogType::None), typeStr("")
{
	ResizeIfNeeded(1024);
}

Log & Pu::Log::GetInstance(void)
{
	static Log logger;
	return logger;
}

void Pu::Log::LogMsgVa(LogType type, bool addNl, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	LogMsg(type, addNl, format, args);
	va_end(args);
}

void Pu::Log::UpdateType(LogType type)
{
	lastType = type;
	WORD typeClr;

	switch (type)
	{
	case (LogType::Debug):
		typeStr = "Debug";
		typeClr = 7;
		break;
	case (LogType::Info):
		typeStr = "Info";
		typeClr = 7;
		break;
	case (LogType::Warning):
		typeStr = "Warning";
		typeClr = 14;
		break;
	case (LogType::Error):
		typeStr = "Error";
		typeClr = 4;
		break;
	default:
		typeStr = "NULL";
		typeClr = 0;
		break;
	}

#if defined(_WIN32)
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), typeClr);
#endif
}

void Pu::Log::LogLinePrefix(LogType type)
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

	/* Update type prefix. */
	if (type != lastType) UpdateType(type);

	/* If any of the funtions in here start to log we end up in an endless loop. */
	suppressLogging = true;

	/* Get process id. */
	const uint64 pid = _CrtGetCurrentProcessId();
	if (processNames.find(pid) == processNames.end()) processNames.emplace(pid, _CrtGetProcessNameFromId(pid));

	/* Get thread id. */
	const uint64 tid = _CrtGetCurrentThreadId();
	if (threadNames.find(tid) == threadNames.end()) threadNames.emplace(tid, _CrtGetThreadNameFromId(tid));

	printf("[%s:%02d][%s/%s][%s]: ", buffer, millisec, processNames.at(pid).c_str(), threadNames.at(tid).c_str(), typeStr);
	suppressLogging = false;
}