#ifdef _WIN32
#include "Core/Platform/Windows/RegistryFetcher.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

/* https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits */
constexpr DWORD MAX_KEY_NAME_SIZE = 255;
constexpr DWORD MAX_VALUE_NAME_SIZE = 16383;

Pu::int32 Pu::RegistryFetcher::ReadInt32(HKEY root, const wstring & subKey, const wstring & value)
{
	int32 result = 0;

	/* Attempt to open a query to the key, return default zero if it failed. */
	const HKEY key = OpenKey(root, subKey, KEY_QUERY_VALUE);
	if (key)
	{
		/* Query the value in the registry. */
		size_t byteSize;
		const DWORD type = QueryValue(key, value, &result, byteSize);

#ifdef _DEBUG
		/* Perform some checks. */
		if (byteSize != 4 && (type != REG_DWORD_LITTLE_ENDIAN && type != REG_DWORD_BIG_ENDIAN))
		{
			Log::Warning("'%ls' is not a DWORD in '%ls'!", value.c_str(), subKey.c_str());
		}
#endif

		RegCloseKey(key);
	}

	return result;
}

Pu::int64 Pu::RegistryFetcher::ReadInt64(HKEY root, const wstring & subKey, const wstring & value)
{
	int64 result = 0;

	/* Attempt to open a query to the key, return default zero if it failed. */
	const HKEY key = OpenKey(root, subKey, KEY_QUERY_VALUE);
	if (key)
	{
		/* Query the value in the registry. */
		size_t byteSize;
		const DWORD type = QueryValue(key, value, &result, byteSize);

#ifdef _DEBUG
		/* Perform some checks on debug mode. */
		if (byteSize != 8 || type != REG_QWORD)
		{
			Log::Warning("'%ls' is not a QWORD in '%ls'!", value.c_str(), subKey.c_str());
		}
#endif

		RegCloseKey(key);
	}

	return result;
}

Pu::wstring Pu::RegistryFetcher::ReadString(HKEY root, const wstring & subKey, const wstring & value)
{
	wstring result;

	/* Attempt to open a query to the key, return default empty if it failed. */
	const HKEY key = OpenKey(root, subKey, KEY_QUERY_VALUE);
	if (key)
	{
		/* Query the size of the string in the registry. */
		size_t byteSize;
		const DWORD type = QueryValue(key, value, nullptr, byteSize);

#ifdef _DEBUG
		/* Perform some checks on debug mode. */
		if (type == REG_SZ || type == REG_EXPAND_SZ)
		{
#endif
			/* Query the value. */
			result.resize(byteSize, L' ');
			QueryValue(key, value, result.data(), byteSize);

#ifdef _DEBUG
		}
		else Log::Warning("'%ls' is not a SZ in '%ls'!", value.c_str(), subKey.c_str());
#endif

		RegCloseKey(key);
	}

	return result;
}

Pu::vector<Pu::wstring> Pu::RegistryFetcher::ReadKeys(HKEY root)
{
	vector<wstring> result;

	/* Attempt to open a query to the key. */
	const HKEY key = OpenKey(root, KEY_ENUMERATE_SUB_KEYS);
	if (key)
	{
		WCHAR name[MAX_KEY_NAME_SIZE];
		LSTATUS state;

		/* Enumerate through the available keys. */
		for (DWORD i = 0;; i++)
		{
			state = RegEnumKey(key, i, name, MAX_KEY_NAME_SIZE);
			if (state == ERROR_SUCCESS) result.emplace_back(name);
			else if (state != ERROR_NO_MORE_ITEMS)
			{
				/* No more items means the end of the keys, only success and this are valid responses. */
				Log::Warning("Sub-key at index %u could not be queried (%ls)!", i, _CrtFormatError(state).c_str());
			}
			else break;
		}

		RegCloseKey(key);
	}

	return result;
}

Pu::vector<Pu::wstring> Pu::RegistryFetcher::ReadKeys(HKEY root, const wstring & subKey)
{
	vector<wstring> result;

	/* Attempt to open a query to the key. */
	const HKEY key = OpenKey(root, subKey, KEY_ENUMERATE_SUB_KEYS);
	if (key)
	{
		WCHAR name[MAX_KEY_NAME_SIZE];
		LSTATUS state;

		/* Enumerate through the available keys. */
		for (DWORD i = 0;; i++)
		{
			state = RegEnumKey(key, i, name, MAX_KEY_NAME_SIZE);
			if (state == ERROR_SUCCESS) result.emplace_back(name);
			else if (state != ERROR_NO_MORE_ITEMS)
			{
				/* No more items means the end of the keys, only success and this are valid responses. */
				Log::Warning("Sub-key at index %u in directory '%ls' could not be queried (%ls)!", i, subKey.c_str(), _CrtFormatError(state).c_str());
			}
			else break;
		}

		RegCloseKey(key);
	}

	return result;
}

Pu::vector<Pu::wstring> Pu::RegistryFetcher::ReadValues(HKEY root, const wstring & subKey)
{
	vector<wstring> result;

	/* Attempt to open a query to the value. */
	const HKEY key = OpenKey(root, subKey, KEY_QUERY_VALUE);
	if (key)
	{
		DWORD dummy = MAX_VALUE_NAME_SIZE;
		WCHAR name[MAX_VALUE_NAME_SIZE];
		LSTATUS state;

		/* Enumerate through the available values. */
		for (DWORD i = 0;; i++, dummy = MAX_VALUE_NAME_SIZE)
		{
			state = RegEnumValue(key, i, name, &dummy, nullptr, nullptr, nullptr, nullptr);
			if (state == ERROR_SUCCESS) result.emplace_back(name);
			else if (state != ERROR_NO_MORE_ITEMS)
			{
				/* No more items means the end of the values, only success and this are valid responses. */
				Log::Warning("Value at index %u in directory '%ls' could not be queried (%ls)!", i, subKey.c_str(), _CrtFormatError(state).c_str());
			}
			else break;
		}

		RegCloseKey(key);
	}

	return result;
}

HKEY Pu::RegistryFetcher::OpenKey(HKEY root, REGSAM permission)
{
	/* Attempt to open a query to the desired root. */
	HKEY result;
	const LSTATUS state = RegOpenKeyEx(root, nullptr, 0, permission, &result);
	if (state == ERROR_SUCCESS) return result;

	/* Handle errors. */
	Log::Error("Unable to open registry key (%ls)!", _CrtFormatError(state).c_str());
	return nullptr;
}

HKEY Pu::RegistryFetcher::OpenKey(HKEY root, const wstring & subKey, REGSAM permission)
{
	/* Attempt to open a query to the desired root. */
	HKEY result;
	const LSTATUS state = RegOpenKeyEx(root, subKey.c_str(), 0, permission, &result);
	if (state == ERROR_SUCCESS) return result;

	/* Handle errors. */
	Log::Error("Unable to open registry key '%ls' (%ls)!", subKey.c_str(), _CrtFormatError(state).c_str());
	return nullptr;
}

DWORD Pu::RegistryFetcher::QueryValue(HKEY key, const wstring & name, void * data, size_t & size)
{
	DWORD type;
	DWORD winSize;

	/* Attempt to query the value. */
	const LSTATUS state = RegQueryValueEx(key, name.c_str(), nullptr, &type, reinterpret_cast<byte*>(data), &winSize);
	if (state == ERROR_SUCCESS)
	{
		size = winSize;
		return type;
	}

	/* Handle errors. */
	Log::Error("Unable to query value for '%ls' (%ls)!", name.c_str(), _CrtFormatError(state).c_str());
	return REG_NONE;
}

#endif