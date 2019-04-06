#include "Core/Platform/DynamicLibLoader.h"
#include "Core/Platform/Windows/Windows.h"
#include "Core/Diagnostics/Logging.h"

#ifdef _WIN32
Pu::DynamicLibLoader::DynamicLibLoader(const wstring & name)
	: hlib(LoadLibrary(name.c_str()))
{
	if (!hlib) Log::Error("Unable to load dynamic library: '%ls'!", name.c_str());
}
#else
Pu::DynamicLibLoader::DynamicLibLoader(const wstring & name)
	: hlib(nullptr)
{
	Log::Error("Unable to load dynamic libraries on this platform!", name.c_str());
}
#endif

Pu::DynamicLibLoader::DynamicLibLoader(DynamicLibLoader && value)
	: hlib(value.hlib)
{
	value.hlib = nullptr;
}

Pu::DynamicLibLoader & Pu::DynamicLibLoader::operator=(DynamicLibLoader && other)
{
	if (this != &other)
	{
		Destroy();
		hlib = other.hlib;
		other.hlib = nullptr;
	}

	return *this;
}

void * Pu::DynamicLibLoader::LoadRawProc(const char * name) const
{
	if (!hlib) return nullptr;

#ifdef _WIN32
	return GetProcAddress(reinterpret_cast<HMODULE>(hlib), name);
#else
	return nullptr;
#endif
}

void Pu::DynamicLibLoader::Destroy()
{
#ifdef _WIN32
	if (hlib) FreeLibrary(reinterpret_cast<HMODULE>(hlib));
#endif
}