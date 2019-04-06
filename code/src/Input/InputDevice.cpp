#include "Input/InputDevice.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

Pu::DynamicLibLoader Pu::InputDevice::hidLibLoader(L"Hid.dll");
Pu::InputDevice::PFN_HidD_GetProductString Pu::InputDevice::HidD_GetProductString = nullptr;

#ifdef _WIN32
/* https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/hidsdi/nf-hidsdi-hidd_getproductstring */
#define MAX_PRODUCT_NAME_LEN  126

Pu::InputDevice::InputDevice(HANDLE hndl, const wstring &deviceInstancePath, InputDeviceType type, const RID_DEVICE_INFO &info)
	: Name(MAX_PRODUCT_NAME_LEN, L' '), Type(type), Info(info), Hndl(hndl), nameSet(false)
{
	Init();

	/* We need to open the file that defines the driver for the HID to get information from it. */
	const HANDLE hHID = CreateFile(deviceInstancePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hHID && hHID != INVALID_HANDLE_VALUE)
	{
		/* Attempt to set a human readable name for the input device. */
		TrySetName(hHID);
		CloseHandle(hHID);
	}
}
#endif

Pu::InputDevice::InputDevice(InputDevice && value)
	: Name(std::move(value.Name)), Type(value.Type), nameSet(value.nameSet)
#ifdef _WIN32
	, Info(std::move(value.Info)), Hndl(value.Hndl)
#endif
{
	value.nameSet = false;

#ifdef _WIN32
	value.Hndl = nullptr;
#endif
}

Pu::InputDevice & Pu::InputDevice::operator=(InputDevice && other)
{
	if (this != &other)
	{
		Destroy();

		Name = std::move(other.Name);
		Type = other.Type;
		nameSet = other.nameSet;
#ifdef _WIN32
		Info = std::move(other.Info);
		Hndl = other.Hndl;
#endif

		other.Hndl = nullptr;
		other.nameSet = false;
	}

	return *this;
}

#ifdef _WIN32
void Pu::InputDevice::TrySetName(HANDLE hHid)
{
	/* We can only get a human readable name if the function is loaded, otherwise just don't set it. */
	const bool result = HidD_GetProductString ? HidD_GetProductString(hHid, Name.data(), static_cast<ULONG>(sizeof(wchar_t) * Name.length())) : false;

	if (result)
	{
		Log::Message("Added HID '%ls'.", Name.c_str());
		nameSet = true;
	}
}
#endif

void Pu::InputDevice::Init()
{
#ifdef _WIN32
	if (!HidD_GetProductString) HidD_GetProductString = hidLibLoader.LoadProc<PFN_HidD_GetProductString>("HidD_GetProductString");
#endif
}

void Pu::InputDevice::Destroy()
{
#ifdef _WIN32
	if (Hndl && nameSet) Log::Message("Removed HID '%ls'.", Name.c_str());
#endif
}