#pragma once
#include "Core/String.h"
#include "Core/Math/Constants.h"
#include "Windows.h"

#ifdef _WIN32
namespace Pu
{
	/* Defines a helper object that handles fetching values from the Windows registry. */
	class RegistryHandler
	{
	public:
		RegistryHandler(void) = delete;
		RegistryHandler(_In_ const RegistryHandler&) = delete;
		RegistryHandler(_In_ RegistryHandler&&) = delete;

		_Check_return_ RegistryHandler& operator =(_In_ const RegistryHandler&) = delete;
		_Check_return_ RegistryHandler& operator =(_In_ RegistryHandler&&) = delete;

		/* Reads the specified value from the sub-key directory in the specified root as a DWORD. */
		_Check_return_ static int32 ReadInt32(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key);
		/* Reads the specified value from the sub-key directory in the specified root as a QWORD. */
		_Check_return_ static int64 ReadInt64(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key);
		/* Reads the specified value from the sub-key directory in the specified root as a string. */
		_Check_return_ static wstring ReadString(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key);
		/* Reads the specified value from the sub-key directory as a DWORD. */
		_Check_return_ static int32 ReadInt32(_In_ HKEY location, _In_ const wstring &key);
		/* Reads the specified value from the sub-key directory as a QWORD. */
		_Check_return_ static int64 ReadInt64(_In_ HKEY location, _In_ const wstring &key);
		/* Reads the specified value from the sub-key directory as a string. */
		_Check_return_ static wstring ReadString(_In_ HKEY location, _In_ const wstring &key);

		/* Attempts to read the specified value from the sub-key directory as a DWORD, if the key could not be found; then create a new key with the specified value. */
		_Check_return_ static int32 TryReadInt32(_In_ HKEY location, _In_ const wstring &key, _In_ int32 value);
		/* Attempts to read the specified value from the sub-key directory as a WWORD, if the key could not be found; then create a new key with the specified value. */
		_Check_return_ static int64 TryReadInt64(_In_ HKEY location, _In_ const wstring &key, _In_ int64 value);
		/* Attempts to read the specified value from the sub-key directory as a string, if the key could not be found; then create a new key with the specified value. */
		_Check_return_ static wstring TryReadString(_In_ HKEY location, _In_ const wstring &key, _In_ const wstring &value);

		/* Queries the sub-keys in the specified root. */
		_Check_return_ static vector<wstring> ReadKeys(_In_ HKEY root);
		/* Queries the sub-keys in the specified root and sub-key directory. */
		_Check_return_ static vector<wstring> ReadKeys(_In_ HKEY root, _In_ const wstring &location);
		/* Queries the values in the specified sub-key. */
		_Check_return_ static vector<wstring> ReadValues(_In_ HKEY root, _In_ const wstring &location);
		/* Checks whether the specified key exists. */
		_Check_return_ static bool Exists(_In_ HKEY root, _In_ const wstring &subKey);
		/* Queries the type of the specified key. */
		_Check_return_ static DWORD QueryType(_In_ HKEY location, _In_ const wstring &key);

		/* Creates the specified sub-key location in the registry (key should be closed with RegCloseKey). */
		_Check_return_ static HKEY Create(_In_ HKEY root, _In_ const wstring &location, REGSAM permission);
		/* Writes the specified integer to the specified key in the registry. */
		static void Write(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key, _In_ int32 value);
		/* Writes the specified integer to the specified key in the registry. */
		static void Write(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key, _In_ int64 value);
		/* Writes the specified string to the specified key in the registry. */
		static void Write(_In_ HKEY root, _In_ const wstring &location, _In_ const wstring &key, _In_ const wstring &value);
		/* Writes the specified integer to the specified key in the registry. */
		static void Write(_In_ HKEY location, _In_ const wstring &key, _In_ int32 value);
		/* Writes the specified integer to the specified key in the registry. */
		static void Write(_In_ HKEY location, _In_ const wstring &key, _In_ int64 value);
		/* Writes the specified string to the specified key in the registry. */
		static void Write(_In_ HKEY location, _In_ const wstring &key, _In_ const wstring &value);

	private:
		static HKEY OpenKey(HKEY root, REGSAM permission);
		static HKEY OpenKey(HKEY root, const wstring &subKey, REGSAM permission);
		static DWORD QueryValue(HKEY key, const wstring &name, void *data, size_t &size, bool raise);
		static void WriteValue(HKEY key, const wstring &name, DWORD type, const void *data, DWORD size);
	};
}
#endif