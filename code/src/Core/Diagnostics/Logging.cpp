#pragma warning(disable:4996)

#include "Core\Diagnostics\Logging.h"
#include <cstdio>
#include <ctime>
#include <crtdbg.h>
#include <mutex>

#if defined(_WIN32)
#include <Windows.h>	// Console colors.
#include <conio.h>		// Press ANY key to continue.
#endif

using namespace Plutonium;

bool shouldAddLinePrefix = true;
LogType lastType = LogType::None;
const char *typeStr;
std::mutex printLock;

void _CrtLogLinePrefix(LogType type)
{
	const time_t now = std::time(nullptr);
	char buffer[100];

	/* Attempt to get the current time. */
	if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now)) == 0) return;

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

#if defined(_WIN32)
	unsigned long pid = GetCurrentProcessId();
#else
	unsigned long pid = 0;
#endif
	
	printf("[%s][%lu/%lu][%s]: ", buffer, pid, _threadid, typeStr);
}

void Plutonium::_CrtLogNoNewLine(LogType type, const char * format, ...)
{
	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0)
	{
		/* Lock output. */
		printLock.lock();

		/* Check if new line needs to be added. */
		if (shouldAddLinePrefix) _CrtLogLinePrefix(type);
		shouldAddLinePrefix = format[len - 1] == '\n';

		/* Log to output. */
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		/* Unlock output. */
		printLock.unlock();
	}
}

void Plutonium::_CrtLog(LogType type, const char * format, ...)
{
	/* Get length and make sure we don't print empty strings. */
	const size_t len = strlen(format);
	if (len > 0)
	{
		/* Lock output. */
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

		/* Unlock output. */
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
	if (len > 0)
	{
		/* Lock output. */
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

		/* Unlock output. */
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