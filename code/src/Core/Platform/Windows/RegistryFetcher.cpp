#ifdef _WIN32
#include "Core/Platform/Windows/Windows.h"
#include "Core/Platform/Windows/RegistryFetcher.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

bool Pu::RegistryFetcher::TryReadInt32(const wstring & key, const wstring & subKey, int32 & result)
{
	size_t size = sizeof(int32);
	return TryReadRaw(key, subKey, &result, &size);
}

bool Pu::RegistryFetcher::TryReadInt64(const wstring & key, const wstring & subKey, int64 & result)
{
	size_t size = sizeof(int64);
	return TryReadRaw(key, subKey, &result, &size);
}

bool Pu::RegistryFetcher::TryReadString(const wstring & key, const wstring & subKey, string & result)
{
	/* Get the required size of the output buffer and early out if this failes. */
	size_t size = 0;
	const bool found = TryReadRaw(key, subKey, nullptr, &size);
	if (!found) return false;

	/* Allocate enought size in buffer and get value. */
	result.reserve(size);
	return TryReadRaw(key, subKey, const_cast<char*>(result.c_str()), &size);
}

bool Pu::RegistryFetcher::TryReadRaw(const wstring & key, const wstring & subKey, void * data, size_t * dataSize)
{
	HKEY readKey;
	LSTATUS result;
	const char *errorMsg;

	/* Try to open a query key to the specified sub key registry. */
	if ((result = RegOpenKeyEx(HKEY_CURRENT_USER, subKey.c_str(), 0, KEY_QUERY_VALUE, &readKey)) == ERROR_SUCCESS)
	{
		/* try to read the desired value from the registry sub key. */
		result = RegQueryValueEx(readKey, key.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(data), (LPDWORD)dataSize);

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
			errorMsg = "Unable to query value '%ls' (%ls)!";
		}
	}
	else errorMsg = "Unable to open registry key '%ls' for query (%ls)!";

	/* Log error. */
	Log::Warning(errorMsg, subKey.c_str(), _CrtFormatError(result).c_str());
	return false;
}
#endif