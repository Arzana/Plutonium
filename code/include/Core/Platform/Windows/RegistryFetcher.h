#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"
#include "Windows.h"

#ifdef _WIN32
namespace Pu
{
	/* Defines a helper object that handles fetching values from the Windows registry. */
	class RegistryFetcher
	{
	public:
		RegistryFetcher(void) = delete;
		RegistryFetcher(_In_ const RegistryFetcher&) = delete;
		RegistryFetcher(_In_ RegistryFetcher&&) = delete;

		_Check_return_ RegistryFetcher& operator =(_In_ const RegistryFetcher&) = delete;
		_Check_return_ RegistryFetcher& operator =(_In_ RegistryFetcher&&) = delete;

		/* Reads the specified value from the sub-key directory in the specified root as a DWORD. */
		_Check_return_ static int32 ReadInt32(_In_ HKEY root, _In_ const wstring &subKey, _In_ const wstring &value);
		/* Reads the specified value from the sub-key directory in the specified root as a QWORD. */
		_Check_return_ static int64 ReadInt64(_In_ HKEY root, _In_ const wstring &subKey, _In_ const wstring &value);
		/* Reads the specified value from the sub-key directory in the specified root as a string. */
		_Check_return_ static wstring ReadString(_In_ HKEY root, _In_ const wstring &subKey, _In_ const wstring &value);

		/* Queries the sub-keys in the specified root. */
		_Check_return_ static vector<wstring> ReadKeys(_In_ HKEY root);
		/* Queries the sub-keys in the specified root and sub-key directory. */
		_Check_return_ static vector<wstring> ReadKeys(_In_ HKEY root, _In_ const wstring &subKey);
		/* Queries the values in the specified sub-key. */
		_Check_return_ static vector<wstring> ReadValues(_In_ HKEY root, _In_ const wstring &subKey);

	private:
		static HKEY OpenKey(HKEY root, REGSAM permission);
		static HKEY OpenKey(HKEY root, const wstring &subKey, REGSAM permission);
		static DWORD QueryValue(HKEY key, const wstring &name, void *data, size_t &size);
	};
}
#endif