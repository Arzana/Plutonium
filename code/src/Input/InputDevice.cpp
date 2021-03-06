#include "Input/InputDevice.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Core/Math/Interpolation.h"

#ifdef _WIN32
/* https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/hidsdi/nf-hidsdi-hidd_getproductstring */
#define HID_MAX_STRING_LENGTH  126
#endif

#define malloc_s(t, s)		reinterpret_cast<t*>(malloc(s))
#define malloc_t(t, a)		malloc_s(t, sizeof(t) * a)

Pu::InputDevice::InputDevice(HANDLE hndl, const wstring &deviceInstancePath, InputDeviceType type, const RID_DEVICE_INFO &info)
	: Type(type), Info(info), Hndl(hndl), btnCnt(0),
	KeyDown("HIDKeyDown"), KeyUp("HIDKeyUp"), ValueChanged("HIDValueChanged")
#ifdef _WIN32
	, Name(HID_MAX_STRING_LENGTH, L' '), Manufacturer(HID_MAX_STRING_LENGTH, L' '),
	data(nullptr)
#endif
{
	/* We can currently only set the identifiers on Windows. */
#ifdef _WIN32
	GetIdentifiers(deviceInstancePath);
#endif

	GetCapacities();
}

float Pu::InputDevice::GetUsageValue(uint16 usageID) const
{
	size_t i = 0;
	for (const ValueInformation &caps : valueCaps)
	{
		if (caps.GetUsageStart() == usageID) return valueStates[i];
		++i;
	}

	return 0.0f;
}

Pu::InputDevice::InputDevice(InputDevice && value)
	: Name(std::move(value.Name)), Manufacturer(std::move(value.Manufacturer)), Type(value.Type), btnCnt(value.btnCnt),
	KeyDown(std::move(value.KeyDown)), KeyUp(std::move(value.KeyUp)), ValueChanged(std::move(value.ValueChanged))
#ifdef _WIN32
	, Info(std::move(value.Info)), Hndl(value.Hndl), data(value.data),
	btnCaps(std::move(value.btnCaps)), tmpUsageList(std::move(value.tmpUsageList)), btnStates(std::move(value.btnStates)),
	valueCaps(std::move(value.valueCaps)), valueStates(std::move(value.valueStates))
#endif
{
#ifdef _WIN32
	value.Hndl = nullptr;
	value.data = nullptr;
#endif
}

Pu::InputDevice & Pu::InputDevice::operator=(InputDevice && other)
{
	if (this != &other)
	{
		Destroy();

		Name = std::move(other.Name);
		Manufacturer = std::move(other.Manufacturer);
		Type = other.Type;
		btnCnt = other.btnCnt;
		KeyDown = std::move(other.KeyDown);
		KeyUp = std::move(other.KeyUp);
		ValueChanged = std::move(other.ValueChanged);

#ifdef _WIN32
		Info = std::move(other.Info);
		Hndl = other.Hndl;
		data = other.data;
		btnCaps = std::move(other.btnCaps);
		tmpUsageList = std::move(other.tmpUsageList);
		btnStates = std::move(other.btnStates);
		valueCaps = std::move(other.valueCaps);
		valueStates = std::move(other.valueStates);

		other.Hndl = nullptr;
		other.data = nullptr;
#endif
	}

	return *this;
}

#ifdef _WIN32
void Pu::InputDevice::SetDefaultName(const wstring & deviceInstancePath)
{
	/* The device instance path will have the following format 
	\\\\?\\type#bus#id#guid for example:
	\\\\?\\HID#SYNHIDMINI&Col02#1&b12c6d1&5&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}
	So we call the HID an unknown device of the device type.
	If this is not the same format than just empty the name.
	*/
	const vector<wstring> parts = deviceInstancePath.split({ L'\\', L'#' });
	if (parts.size() > 1)
	{
		Name = L"Unknown " + parts[1] + L" device";
		Log::Message("Added %ls", Name.c_str());
	}
	else Name.clear();
}

void Pu::InputDevice::HandleWin32Event(const RAWHID & info)
{
	PCHAR rawData = reinterpret_cast<PCHAR>(const_cast<PBYTE>(info.bRawData));

	/* Check the button state for each capacity object. */
	uint16 i = 0;
	for (ButtonInformation &caps : btnCaps)
	{
		vector<bool> states(btnStates[i].size());

		/* Get the ids of the buttons pressed. */
		ULONG size = static_cast<ULONG>(btnCnt);
		if (HidP_GetUsages(HidP_Input, caps.page, 0, tmpUsageList.data(), &size, data, rawData, info.dwSizeHid) == HIDP_STATUS_SUCCESS)
		{
			/* Check for pressed buttons. */
			for (ULONG j = 0; j < size; j++)
			{
				/* Make sure that we send the mesage for the correct button range. */
				USAGE cur = tmpUsageList[j];
				if (caps.min <= cur && caps.max >= cur)
				{
					const ButtonEventArgs args(caps, cur - caps.min);
					states[args.KeyCode] = true;
					KeyDown.Post(*this, args);
				}
			}

			/* Check for released buttons. */
			for (uint16 j = 0; j < btnStates[i].size(); j++)
			{
				if (btnStates[i][j]) KeyUp.Post(*this, ButtonEventArgs(caps, j));
			}

			/* Update the old state. */
			btnStates[i].assign(states.begin(), states.end());
		}
	}

	/* Check the value state for each capacity object. */
	i = 0;
	for (ValueInformation &caps : valueCaps)
	{
		/* Get the value of the specified slider. */
		ULONG value;
		if (HidP_GetUsageValue(HidP_Input, caps.page, 0, caps.min, &value, data, rawData, info.dwSizeHid) == HIDP_STATUS_SUCCESS)
		{
			/* Scale the value to a normalized ([0, 1]) range. */
			const float newValue = ilerp(caps.valueMin, caps.valueMax, static_cast<float>(value));

			/* Check if the value differs and post if a change was made. */
			if (newValue != valueStates[i])
			{
				ValueChanged.Post(*this, ValueEventArgs(caps, value, newValue));
				valueStates[i] = newValue;
			}
		}

		i++;
	}
}

void Pu::InputDevice::GetIdentifiers(const wstring & deviceInstancePath)
{
	/*
	We need to open the file that defines the information for the HID to get information from it.
	We only need to read from this so just enable GENERIC_READ only.
	We also don't care if any other process reads or writes to the file whilst were reading so defined share access of read/write.
	Some devices even seem to need FILE_SHARE_WRITE in the share mode to get the name at all (I don't know why).
	*/
	const HANDLE hHID = CreateFile(deviceInstancePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hHID && hHID != INVALID_HANDLE_VALUE)
	{
		/* Attempt to set a human readable name for the input device. */
		if (HidD_GetProductString(hHID, Name.data(), HID_MAX_STRING_LENGTH)) Log::Message("Added HID '%ls'.", Name.c_str());
		else SetDefaultName(deviceInstancePath);

		/* Attempt to set the manufacturer name (almost always set for external devices), otherwise just set it to an empty string. */
		if (!HidD_GetManufacturerString(hHID, Manufacturer.data(), HID_MAX_STRING_LENGTH)) Manufacturer = L"";

		CloseHandle(hHID);
	}
	else
	{
		SetDefaultName(deviceInstancePath);
		Manufacturer = L"";
	}
}
#endif

void Pu::InputDevice::GetCapacities(void)
{
#ifdef _WIN32
	/* We'll get all of our capacities through pre-parsed data so make sure there is any. */
	uint32 preParsedDataSize;
	if (GetRawInputDeviceInfo(Hndl, RIDI_PREPARSEDDATA, nullptr, &preParsedDataSize) != 0) return;

	/* Early out if there is no preparsed data (i.e. no buttons or value sliders). */
	if (!preParsedDataSize) return;

	/* Actually get the pre-parsed data. */
	data = malloc_s(_HIDP_PREPARSED_DATA, preParsedDataSize);
	GetRawInputDeviceInfo(Hndl, RIDI_PREPARSEDDATA, data, &preParsedDataSize);

	/* 
	Attempt to get the global capacities of the device. 
	The only status codes that these functions return are SUCCESS
	and INVALID_PREPARSED_DATA so we just have to check for SUCCESS.
	*/
	HIDP_CAPS caps;
	if (HidP_GetCaps(data, &caps) == HIDP_STATUS_SUCCESS)
	{
		/* Get the button capacities. */
		if (caps.NumberInputButtonCaps > 0)
		{
			/* Make sure to only allocate the buffer if we actually have any buttons. */
			PHIDP_BUTTON_CAPS rawBtnCaps = malloc_t(HIDP_BUTTON_CAPS, caps.NumberInputButtonCaps);
			if (HidP_GetButtonCaps(HidP_Input, rawBtnCaps, &caps.NumberInputButtonCaps, data) == HIDP_STATUS_SUCCESS)
			{
				/* Preallocate the value buffer and set the button count. */
				for (USHORT i = 0; i < caps.NumberInputButtonCaps; i++)
				{
					const USHORT cnt = rawBtnCaps[i].Range.UsageMax - rawBtnCaps[i].Range.UsageMin + 1;
					btnCaps.emplace_back(ButtonInformation(rawBtnCaps[i]));
					btnStates.emplace_back(cnt);

					btnCnt += cnt;
				}
			}
			else FailedCapacities("invalid preparsed data for button capacities");

			free(rawBtnCaps);
		}

		/* Get the value capacities. */
		if (caps.NumberInputValueCaps > 0)
		{
			/* Make sure to only allocate the buffer if we have values. */
			PHIDP_VALUE_CAPS rawValueCaps = malloc_t(HIDP_VALUE_CAPS, caps.NumberInputValueCaps);
			valueStates.resize(caps.NumberInputValueCaps);

			if (HidP_GetValueCaps(HidP_Input, rawValueCaps, &caps.NumberInputValueCaps, data) == HIDP_STATUS_SUCCESS)
			{
				for (USHORT i = 0; i < caps.NumberInputValueCaps; i++)
				{
					valueCaps.emplace_back(ValueInformation(rawValueCaps[i]));
				}
			}
			else FailedCapacities("invalid preparsed data for value capacities");

			free(rawValueCaps);
		}

		/* Preallocate a temporary buffer used to compare states. */
		tmpUsageList.resize(btnCnt);
	}
	else FailedCapacities("preparsed data is invalid");
#endif
}

void Pu::InputDevice::FailedCapacities(const char * reason) const
{
	Log::Warning("Could not get device capacities for '%ls' (%s)!", Name.c_str(), reason);
}

void Pu::InputDevice::Destroy(void)
{
#ifdef _WIN32
	if (Hndl) Log::Message("Removed HID '%ls'.", Name.c_str());
	if (data) free(data);
#endif
}