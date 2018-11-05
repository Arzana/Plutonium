#include "Core/Platform/Windows/RegistryFetcher.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

#ifdef _WIN32
#include <Windows.h>

bool Pu::RegistryFetcher::TryReadInt32(const char * key, const char * subKey, int32 & result)
{
	size_t size = sizeof(int32);
	return TryReadRaw(key, subKey, &result, &size);
}

bool Pu::RegistryFetcher::TryReadInt64(const char * key, const char * subKey, int64 & result)
{
	size_t size = sizeof(int64);
	return TryReadRaw(key, subKey, &result, &size);
}

bool Pu::RegistryFetcher::TryReadString(const char * key, const char * subKey, string & result)
{
	/* Get the required size of the output buffer and early out if this failes. */
	size_t size = 0;
	const bool found = TryReadRaw(key, subKey, nullptr, &size);
	if (!found) return false;

	/* Allocate enought size in buffer and get value. */
	result.reserve(size);
	return TryReadRaw(key, subKey, const_cast<char*>(result.c_str()), &size);
}

bool Pu::RegistryFetcher::TryReadRaw(const char * key, const char * subKey, void * data, size_t * dataSize)
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
	string error = Pu::_CrtFormatError(result);
	Log::Warning(errorMsg, subKey, error.c_str());
	return false;
}
#endif