#include "Streams\FileUtils.h"
#include "Core\Diagnostics\Logging.h"
#include "Core\Diagnostics\StackTrace.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(linux)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

bool Plutonium::_CrtDirectoryExists(const char * directory)
{
#if defined(_WIN32)

	/* Get file attributes, if it isn't invalid and is defined as a direcory. */
	DWORD attrib = GetFileAttributes(directory);
	return (attrib != INVALID_FILE_ATTRIBUTES && ((attrib & FILE_ATTRIBUTE_DIRECTORY) != 0));

#elif defined(linux)

	/* Attempt to access the file. */
	if (!access(directory, 0))
	{
		/* Get file attributes. */
		stat status;
		stat(directory, &status);

		/* If mode is directory. */
		return (status.st_mode & S_IFDIR) != 0;
	}

	return false;

#else

	LOG_WAR_ONCE("Checking if a directory excists is not supported on this platform!");
	return false;

#endif
}

void Plutonium::_CrtCreateDirectory(const char * directory)
{
	if (strlen(directory) < 1) return;

#if defined(_WIN32)

	/* Attempt to create the directory and throw (on debug) is failed. */
	bool successful = CreateDirectory(directory, nullptr);
	LOG_THROW_IF(!successful, "Unable to create directory '%s', reason: %s!", directory, _CrtGetErrorString());

#else

	LOG_WAR_ONCE("Creating a directory is not supported on this platform!");

#endif
}