#pragma warning(disable:4996)

#include "Core\Diagnostics\Logging.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\Threading\ThreadUtils.h"
#include "Core\SafeMemory.h"
#include <map>
#include <cstdio>
#include <ctime>
#include <crtdbg.h>
#include <mutex>

#if defined(_WIN32)
#include <Windows.h>	// Console colors.
#include <conio.h>		// Press ANY key to continue.
#endif

using namespace Plutonium;

using KvP = std::pair<uint64, const char*>;

bool shouldAddLinePrefix = true;
bool suppressLogging = false;
LogType lastType = LogType::None;
const char *typeStr;
std::map<uint64, const char*> processNames;
std::map<uint64, const char*> threadNames;
std::mutex printLock;

void _CrtUpdateType(LogType type)
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

void _CrtLogLinePrefix(LogType type)
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
	if (type != lastType) _CrtUpdateType(type);

	/* If any of the funtions in here start to log we end up in an endless loop. */
	suppressLogging = true;

	/* Get process id. */
	uint64 pid = _CrtGetCurrentProcessId();
	if (processNames.find(pid) == processNames.end()) processNames.insert(KvP(pid, _CrtGetProcessNameFromId(pid)));

	/* Get thread id. */
	uint64 tid = _CrtGetCurrentThreadId();
	if (threadNames.find(tid) == threadNames.end()) threadNames.insert(KvP(tid, _CrtGetThreadNameFromId(tid)));

	printf("[%s:%02d][%s/%s][%s]: ", buffer, millisec, processNames.at(pid), threadNames.at(tid), typeStr);
	suppressLogging = false;
}

void Plutonium::_CrtFinalizeLog(void)
{
	/* Release process names. */
	for (std::map<uint64, const char*>::iterator it = processNames.begin(); it != processNames.end(); it++)
	{
		free_s(it->second);
	}

	/* Release thread names. */
	for (std::map<uint64, const char*>::iterator it = threadNames.begin(); it != threadNames.end(); it++)
	{
		free_s(it->second);
	}

	processNames.clear();
	threadNames.clear();
}

void Plutonium::_CrtResizeConsoleIfNeeded(size_t width)
{
#if defined(_WIN32)
	/* Get terminal handle. */
	HANDLE terminalHndl = GetStdHandle(STD_OUTPUT_HANDLE);

	/* Get current buffer info. */
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(terminalHndl, &info)) return;

	/* Calculate required width. */
	SHORT x = info.dwCursorPosition.X + static_cast<SHORT>(width);
	if (x > info.dwSize.X)
	{
		/* Increase terminal buffer size. */
		SetConsoleScreenBufferSize(terminalHndl, { x, info.dwSize.Y });
	}
#else
	LOG_WAR_ONCE("Cannot resize console buffer on this platform!");
#endif
}

void Plutonium::_CrtLogNoNewLine(LogType type, const char * format, ...)
{

	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0 && !suppressLogging)
	{
		/* Lock logger. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) _CrtLogLinePrefix(type);
		shouldAddLinePrefix = format[len - 1] == '\n';

		/* Log to output. */
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		/* Unlock logger. */
		printLock.unlock();
	}
}

void Plutonium::_CrtLog(LogType type, const char * format, ...)
{
	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0 && !suppressLogging)
	{
		/* Lock logger. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) _CrtLogLinePrefix(type);
		shouldAddLinePrefix = true;

		/* Log to output. */
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		/* Add newline if needed. */
		if (format[len - 1] != '\n') printf("\n");

		/* Unlock logger. */
		printLock.unlock();
	}
}

void Plutonium::_CrtLogExc(const char * sender, const char * file, const char * func, int line)
{
	/* Log error header. */
	_CrtLogNoNewLine(LogType::Error, "%s threw an exception!\nFILE:		%s.\nFUNCTION:	%s.\nLINE:		%d.\nMESSAGE:	",
		sender ? sender : "Undefined caller", file, func, line);
}

void Plutonium::_CrtLogExc(unsigned int framesToSkip)
{
	std::vector<const StackFrame*> callStack = _CrtGetStackTrace(framesToSkip);

	/* Lock logger. */
	printLock.lock();

	/* Make sure the color is correct. */
	_CrtUpdateType(LogType::Error);

	/* Log table header. */
	printf("STACKTRACE:\n");
	printf("		%46s FUNCTION %54s LINE %8s MODULE %16s FILE\n", "", "", "", "");

	bool suppressLog = false;
	for (size_t i = 0; i < callStack.size(); i++)
	{
		const StackFrame *cur = callStack.at(i);

		if (!suppressLog)
		{
			/* Log stack trace. */
			printf("		at %-102s ", cur->FunctionName);
			printf(cur->Line ? "| %-10d " : "| Unknown    ", cur->Line);
			printf("| %-16s", cur->ModuleName);
			printf("| %-64s\n", cur->FileName);
			
			/* Stop stacktrace log after either a thread start has been found or main has been found. */
			if (strstr(cur->FunctionName, "_CrtPuThreadStart")) suppressLog = true;
			if (!strcmp(cur->FunctionName, "main")) suppressLog = true;
			if (suppressLog) printf("		[External Code]\n");
		}

		delete_s(cur);
	}

	/* Unlock logger. */
	printLock.unlock();
}

void Plutonium::_CrtLogThrow(const char * msg, const char * file, const char * func, int line, const char * desc, ...)
{
	/* log error header. */
	_CrtLogExc(nullptr, file, func, line);

	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(desc);
	if (len > 0 && !suppressLogging)
	{
		/* Lock logger. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) _CrtLogLinePrefix(LogType::Error);
		shouldAddLinePrefix = true;

		/* Log to output. */
		va_list args;
		va_start(args, desc);
		vprintf(desc, args);
		va_end(args);

		/* Add newline if needed. */
		if (desc[len - 1] != '\n') printf("\n");

		/* Unlock logger. */
		printLock.unlock();
	}

	/*
	Log error footer.
	Skip last three frames:
	- _CrtGetStackTrace
	- _CrtLogExc
	- _CrtLogThrow
	*/
	_CrtLogExc(3);

	/* Make sure we halt excecution for all threads. */
	throw std::exception(msg);
}

bool Plutonium::_CrtLogBacktrack(size_t amnt)
{
#if defined(_WIN32)
	/* Get console handle. */
	HANDLE hndlr = GetStdHandle(STD_OUTPUT_HANDLE);
	/* Get current position. */
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hndlr, &csbi)) return false;

	/* Alter position. */
	csbi.dwCursorPosition.X -= static_cast<SHORT>(amnt);
	printLock.lock();
	SetConsoleCursorPosition(hndlr, csbi.dwCursorPosition);
	printLock.unlock();
	return true;
#else
	LOG_WAR("Backtracking the output is not supported on this platform!");
	return false;
#endif
}

void Plutonium::_CrtPressAnyKeyToContinue(void)
{
#if defined(_WIN32)
	LOG_MSG("Press any key to continue...");
	getch();
#else
	LOG_MSG("Press ENTER to continue...");
	getchar();
#endif
}