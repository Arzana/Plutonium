#include "Core\Platform\Windows\RegistryFetcher.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\SafeMemory.h"

#if defined (_WIN32)
#include <Windows.h>

bool Plutonium::RegistryFetcher::TryReadInt32(const char * key, const char * subKey, int32 * result)
{
	/* We don't care about the size if we're reading a DWORD value. */
	size_t size = sizeof(int32);
	return TryReadRaw(key, subKey, result, &size);
}

bool Plutonium::RegistryFetcher::TryReadInt64(const char * key, const char * subKey, int64 * result)
{
	/* We don't care about the size if we're reading a QWORD value. */
	size_t size = sizeof(int64);
	return TryReadRaw(key, subKey, result, &size);
}

bool Plutonium::RegistryFetcher::TryReadString(const char * key, const char * subKey, const char ** result)
{
	/* Get the required size of the output buffer and early out if this failes. */
	size_t size = 0;
	bool found = TryReadRaw(key, subKey, nullptr, &size);
	if (!found) return false;

	/* Allocate enought size in buffer and get value. */
	*result = malloc_s(char, size);
	return TryReadRaw(key, subKey, const_cast<char*>(*result), &size);
}

bool Plutonium::RegistryFetcher::TryReadRaw(const char * key, const char * subKey, void * data, size_t *dataSize)
{
	HKEY readKey;
	LSTATUS result;
	const char *errorMsg;

	/* Try to open a query key to the specified sub key registry. */
	if ((result = RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_QUERY_VALUE, &readKey)) == ERROR_SUCCESS)
	{
		/* try to read the desired value from the registry sub key. */
		result = RegQueryValueExA(readKey, key, nullptr, nullptr, reinterpret_cast<LPBYTE>(data), (LPDWORD)dataSize);

		if (result == ERROR_SUCCESS)
		{
			/* Retruns and closes the sub key. */
			RegCloseKey(readKey);
			return true;
		}
		else
		{
			/* Sets error message and closes the sub key. */
			RegCloseKey(readKey);
			errorMsg = "Unable to query value '%s' (%s)!";
		}
	}
	else errorMsg = "Unable to open registry key '%s' for query (%s)!";

	/* Log error. */
	const char *error = Plutonium::_CrtFormatError(result);
	LOG_WAR(errorMsg, subKey, error);
	free_s(error);
	return false;
}

#endif