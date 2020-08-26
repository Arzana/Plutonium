#include "Streams/RuntimeConfig.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/ThreadUtils.h"

#ifdef _WIN32
#include "Core/Platform/Windows/RegistryHandler.h"
#include "Core/Math/Conversion.h"

constexpr const wchar_t *BOOL_PREFIX = L"Bool_";
constexpr const wchar_t *INT_PREFIX = L"Int32_";
constexpr const wchar_t *LONG_PREFIX = L"Int64_";
constexpr const wchar_t *SINGLE_PREFIX = L"Float_";
constexpr const wchar_t *STRING_PREFIX = L"String_";

static HKEY hroot;
static void pu_reg_init_win32(const Pu::wstring &processName)
{
	/* Precreate the configuration root. */
	Pu::wstring root = L"Software\\Plutonium\\";
	hroot = Pu::RegistryHandler::Create(HKEY_CURRENT_USER, root + processName, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS);
	if (!hroot) Pu::Log::Fatal("Unable to initialize runtime config!");
}

#else
#define NOT_IMPLEMENTED_MSG(func)		#func " is not implemented on this platform!"
#define NOT_IMPLEMENTED_ERROR(func)		Log::Error(NOT_IMPLEMENTED_MSG(func))
#define NOT_IMPLEMENTED_FATAL(func)		Log::Fatal(NOT_IMPLEMENTED_MSG(func))
#endif

static void pu_config_init()
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	const Pu::wstring &process = Pu::_CrtGetProcessNameFromId(Pu::_CrtGetCurrentProcessId());

#ifdef _WIN32
	pu_reg_init_win32(process);
#endif
}

Pu::wstring Pu::RuntimeConfig::QueryHumanReadable(const wstring & key)
{
	pu_config_init();

#ifdef _WIN32
	if (RegistryHandler::Exists(hroot, BOOL_PREFIX + key))
	{
		return QueryBool(key) ? L"True" : L"False";
	}

	if (RegistryHandler::Exists(hroot, INT_PREFIX + key))
	{
		return wstring::from(QueryInt(key));
	}

	if (RegistryHandler::Exists(hroot, LONG_PREFIX + key))
	{
		return wstring::from(QueryLong(key));
	}

	if (RegistryHandler::Exists(hroot, SINGLE_PREFIX + key))
	{
		return QueryStringInternal(SINGLE_PREFIX, key, L"");
	}

	if (RegistryHandler::Exists(hroot, STRING_PREFIX + key))
	{
		return QueryString(key);
	}

	return L"<Unknown Key>";
#else
	NOT_IMPLEMENTED_ERROR(QueryHumanReadable);
	return "NULL";
#endif
}

Pu::vector<Pu::wstring> Pu::RuntimeConfig::QueryKeys(void)
{
	pu_config_init();

#ifdef _WIN32
	/* Remove the prefixes from the result. */
	vector<wstring> result = RegistryHandler::ReadValues(hroot, L"");
	for (wstring &cur : result) cur = cur.trim_front_split(L'_');
	return result;
#else
	NOT_IMPLEMENTED_ERROR(QueryKeys);
	return vector<wstring>();
#endif
}

bool Pu::RuntimeConfig::QueryBool(const wstring & key, bool defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadInt32(hroot, BOOL_PREFIX + key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryBool);
	return false;
#endif
}

Pu::int32 Pu::RuntimeConfig::QueryInt(const wstring & key, int32 defaultValue)
{
	return QueryIntInternal(INT_PREFIX, key, defaultValue);
}

Pu::int64 Pu::RuntimeConfig::QueryLong(const wstring & key, int64 defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadInt64(hroot, LONG_PREFIX + key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryLong);
	return 0;
#endif
}

float Pu::RuntimeConfig::QuerySingle(const wstring & key, float defaultValue)
{
	const string raw = QueryStringInternal(SINGLE_PREFIX, key, wstring::from(defaultValue)).toUTF8();

	float result;
	if (tryParse(raw.c_str(), raw.c_str() + raw.length(), &result)) return result;
	return NAN;
}

Pu::wstring Pu::RuntimeConfig::QueryString(const wstring & key, const wstring & defaultValue)
{
	return QueryStringInternal(STRING_PREFIX, key, defaultValue);
}

void Pu::RuntimeConfig::Set(const wstring & key, bool value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, BOOL_PREFIX + key, value ? 1 : 0);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, int32 value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, INT_PREFIX + key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, int64 value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, LONG_PREFIX + key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, float value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, SINGLE_PREFIX + key, wstring::from(value));
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, const wstring & value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, STRING_PREFIX + key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

Pu::int32 Pu::RuntimeConfig::QueryIntInternal(const wchar_t * prefix, const wstring & key, int32 defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadInt32(hroot, prefix + key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryInt);
	return 0;
#endif
}

Pu::wstring Pu::RuntimeConfig::QueryStringInternal(const wchar_t * prefix, const wstring & key, const wstring & defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadString(hroot, prefix + key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryString);
	return L"";
#endif
}