#pragma warning(disable:4996)

#include "Core\Diagnostics\Logging.h"
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
	if (type != lastType)
	{
		lastType = type;
		int typeClr;

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
		free_cstr_s(it->second);
	}

	/* Release thread names. */
	for (std::map<uint64, const char*>::iterator it = threadNames.begin(); it != threadNames.end(); it++)
	{
		free_cstr_s(it->second);
	}

	processNames.clear();
	threadNames.clear();
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

	/* On debug mode throw error window with info; on release just throw. */
#if defined(DEBUG)
	_CrtDbgReport(_CRT_ERROR, file, line, nullptr, msg);
	_CrtDbgBreak();
#else
	throw;
#endif
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