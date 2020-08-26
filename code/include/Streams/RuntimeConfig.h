#pragma once
#include "Core/String.h"
#include "Core/EnumUtils.h"

namespace Pu
{
	/* Defines an object used to query configurations from a persistent data store. */
	class RuntimeConfig
	{
	public:
		RuntimeConfig(_In_ const RuntimeConfig&) = delete;
		RuntimeConfig(_In_ RuntimeConfig&&) = delete;

		_Check_return_ RuntimeConfig& operator =(_In_ const RuntimeConfig&) = delete;
		_Check_return_ RuntimeConfig& operator =(_In_ RuntimeConfig&&) = delete;

		/* Gets a human readable version of the specified configuration value. */
		_Check_return_ static wstring QueryHumanReadable(_In_ const wstring &key);
		/* Queries a list of all the defined keys in the configuration. */
		_Check_return_ static vector<wstring> QueryKeys(void);

		/* Queries a specified boolean configuration value. */
		_Check_return_ static bool QueryBool(_In_ const wstring &key, _In_ bool defaultValue = false);
		/* Queries a specified integer configuration value. */
		_Check_return_ static int32 QueryInt(_In_ const wstring &key, _In_ int32 defaultValue = 0);
		/* Queries a specified long integer configuration value. */
		_Check_return_ static int64 QueryLong(_In_ const wstring &key, _In_ int64 defaultValue = 0);
		/* Queries a specified floating point configuration value. */
		_Check_return_ static float QuerySingle(_In_ const wstring &key, _In_ float defaultValue = 0.0f);
		/* Queries a specified string configuration value. */
		_Check_return_ static wstring QueryString(_In_ const wstring &key, _In_ const wstring &defaultValue = L"");

		/* Queries a specified enum configuration value. */
		template <typename enum_t>
		_Check_return_ static inline enum_t QueryEnum(_In_ const wstring &key, _In_ enum_t defaultValue)
		{
			return _CrtInt2Enum<enum_t>(QueryIntInternal((_CrtEnumGetName<enum_t>().toWide() + L'_').c_str(), key, static_cast<int32>(defaultValue)));
		}

		/* Sets a specified boolean configuration value. */
		static void Set(_In_ const wstring &key, _In_ bool value);
		/* Sets a specified integer configuration value. */
		static void Set(_In_ const wstring &key, _In_ int32 value);
		/* Sets a specified long integer configuration value. */
		static void Set(_In_ const wstring &key, _In_ int64 value);
		/* Sets a specified floating point configuration value. */
		static void Set(_In_ const wstring &key, _In_ float value);
		/* Sets a specified string configuration value. */
		static void Set(_In_ const wstring &key, _In_ const wstring &value);

	private:
		static int32 QueryIntInternal(const wchar_t *prefix, const wstring &key, int32 defaultValue);
		static wstring QueryStringInternal(const wchar_t *prefix, const wstring &key, const wstring &defaultValue);
	};
}