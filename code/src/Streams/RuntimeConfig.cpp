#include "Streams/RuntimeConfig.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/ThreadUtils.h"

#ifdef _WIN32
#include "Core/Platform/Windows/RegistryHandler.h"
#include "Core/Math/Conversion.h"

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
	const DWORD type = RegistryHandler::QueryType(hroot, key);

	switch (type)
	{
	case REG_DWORD:
		return wstring::from(QueryInt(key, 0));
	case REG_QWORD:
		return wstring::from(QueryLong(key, 0ll));
	case REG_SZ:
		return QueryString(key, L"");
	default:
		Log::Error("Unable to parse registry to %u to string!", type);
		return L"Undefined";
	}
#else
	NOT_IMPLEMENTED_ERROR(QueryHumanReadable);
	return "NULL";
#endif
}

Pu::vector<Pu::wstring> Pu::RuntimeConfig::QueryKeys(void)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::ReadKeys(hroot);
#else
	NOT_IMPLEMENTED_ERROR(QueryKeys);
	return vector<wstring>();
#endif
}

bool Pu::RuntimeConfig::QueryBool(const wstring & key, bool defaultValue)
{
	return QueryInt(key, defaultValue);
}

Pu::int32 Pu::RuntimeConfig::QueryInt(const wstring & key, int32 defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadInt32(hroot, key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryInt);
	return 0;
#endif
}

Pu::int64 Pu::RuntimeConfig::QueryLong(const wstring & key, int64 defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadInt64(hroot, key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryLong);
	return 0;
#endif
}

float Pu::RuntimeConfig::QuerySingle(const wstring & key, float defaultValue)
{
	const string raw = QueryString(key, wstring::from(defaultValue)).toUTF8();

	float result;
	if (tryParse(raw.c_str(), raw.c_str() + raw.length(), &result)) return result;
	return NAN;
}

Pu::wstring Pu::RuntimeConfig::QueryString(const wstring & key, const wstring & defaultValue)
{
	pu_config_init();

#ifdef _WIN32
	return RegistryHandler::TryReadString(hroot, key, defaultValue);
#else
	NOT_IMPLEMENTED_FATAL(QueryString);
	return 0;
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, bool value)
{
	Set(key, value ? 1 : 0);
}

void Pu::RuntimeConfig::Set(const wstring & key, int32 value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, int64 value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}

void Pu::RuntimeConfig::Set(const wstring & key, float value)
{
	Set(key, wstring::from(value));
}

void Pu::RuntimeConfig::Set(const wstring & key, const wstring & value)
{
	pu_config_init();

#ifdef _WIN32
	RegistryHandler::Write(hroot, key, value);
#else
	NOT_IMPLEMENTED_ERROR(Set);
#endif
}