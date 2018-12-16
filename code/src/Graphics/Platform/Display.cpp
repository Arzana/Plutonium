#include "Graphics/Platform/Display.h"
#include "Core/Diagnostics/DbgUtils.h"

static Pu::vector<Pu::Display> availableDisplays;

Pu::Display::Display(const Display & value)
	: name(value.name), viewport(value.viewport), hertz(value.hertz),
	depth(value.depth), isPrimary(value.isPrimary), correction(value.correction)
{}

Pu::Display::Display(Display && value)
	: name(std::move(value.name)), viewport(value.viewport), hertz(value.hertz),
	depth(value.depth), isPrimary(value.isPrimary), correction(value.correction)
{}

Pu::Display & Pu::Display::operator=(const Display & other)
{
	if (this != &other)
	{
		name = other.name;
		viewport = other.viewport;
		hertz = other.hertz;
		depth = other.depth;
		isPrimary = other.isPrimary;
		correction = other.correction;
	}

	return *this;
}

Pu::Display & Pu::Display::operator=(Display && other)
{
	if (this != &other)
	{
		name = std::move(other.name);
		viewport = other.viewport;
		hertz = other.hertz;
		depth = other.depth;
		isPrimary = other.isPrimary;
		correction = other.correction;
	}

	return *this;
}

const Pu::Display & Pu::Display::GetPrimaryDisplay(void)
{
	FindDisplays();

	for (const Display &cur : availableDisplays)
	{
		if (cur.isPrimary) return cur;
	}

	Log::Warning("Unable to find primary display, returning empty display!");
	return Display();
}

const Pu::Display & Pu::Display::GetDisplayAt(Offset2D point)
{
	FindDisplays();

	for (const Display &cur : availableDisplays)
	{
		if (cur.viewport.Contains(point)) return cur;
	}

	Log::Warning("Unable to find display at [%d, %d], returning empty display!", point.X, point.Y);
	return Display();
}

const Pu::vector<Pu::Display>& Pu::Display::GetAll(void)
{
	FindDisplays();
	return availableDisplays;
}

Pu::Display::Display(void)
	: depth(0), hertz(0), isPrimary(false)
{}

void Pu::Display::FindDisplays(void)
{
	/* Make sure this is not called every time a display is requested. */
	static bool called = false;
	if (called) return;
	called = true;

	availableDisplays.clear();

#ifdef _WIN32
	if (!EnumDisplayMonitors(nullptr, nullptr, &Display::MonitorProc, 0))
	{
		const string error = _CrtGetErrorString();
		Log::Error("Unable to get physical monitors, reason: '%s'!", error.c_str());
}
#else
	Log::Warning("Unable to get monitor information on this platform!");
#endif
}

#ifdef _WIN32
BOOL Pu::Display::MonitorProc(HMONITOR monitor, HDC, LPRECT vp, LPARAM)
{
	Display result;

	/* Get monitor viewport. */
	const Offset2D pos(static_cast<int32>(vp->left), static_cast<int32>(vp->top));
	const Extent2D size(static_cast<uint32>(vp->right - vp->left), static_cast<uint32>(vp->bottom - vp->top));
	result.viewport = Rect2D(pos, size);

	/* Get visible information about the monitor. */
	MONITORINFOEX info = { sizeof(MONITORINFOEX) };
	if (GetMonitorInfo(monitor, &info))
	{
		/* Check if this is the primary monitor. */
		result.isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY;

		/* Get human readable monitor name. */
		DISPLAY_DEVICE device = { sizeof(DISPLAY_DEVICE) };
		for (DWORD monitorIdx = 0; EnumDisplayDevices(info.szDevice, monitorIdx, &device, 0); monitorIdx++)
		{
			if (result.name.length()) result.name += ' ';
			result.name += device.DeviceString;
		}

		/* Get refresh rate and color depth. */
		DEVMODE settings;
		settings.dmSize = sizeof(DEVMODE);
		if (EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &settings))
		{
			result.hertz = static_cast<uint32>(settings.dmDisplayFrequency);
			result.depth = static_cast<uint32>(settings.dmBitsPerPel);
		}
		else
		{
			const string error = _CrtGetErrorString();
			Log::Error("Unable to get display settings for '%s'!", result.name.c_str());
		}

		/* Create context for the monitor. */
		const HDC hdc = CreateDC(nullptr, info.szDevice, nullptr, nullptr);
		if (hdc)
		{
			constexpr size_t ELEM_CNT = 256;
			constexpr float MULTIPLIER = recip(ELEM_CNT * 3.0f);

			/* Create temporary structure for gamma ramp result. */
			struct RAMP
			{
				WORD red[ELEM_CNT];
				WORD green[ELEM_CNT];
				WORD blue[ELEM_CNT];
			} ramp;

			/* Get gamma ramp. */
			if (GetDeviceGammaRamp(hdc, &ramp))
			{
				/* Get average gamma correction. */
				float sum = 0.0f;
				for (size_t i = 0; i < ELEM_CNT; i++)
				{
					const uint32 power = static_cast<uint32>(i + 1);
					sum += nthrt(static_cast<float>(ramp.red[i]), power);
					sum += nthrt(static_cast<float>(ramp.green[i]), power);
					sum += nthrt(static_cast<float>(ramp.blue[i]), power);
				}

				result.correction = recip(sum * MULTIPLIER);
			}
			else
			{
				const string error = _CrtGetErrorString();
				Log::Error("Unable to get gamma ramp for display '%s'!", result.name.c_str());
			}
		}
		else
		{
			const string error = _CrtGetErrorString();
			Log::Error("Unable to create HDC for display '%s'!", result.name.c_str());
		}
	}
	else
	{
		const string error = _CrtGetErrorString();
		Log::Error("Unable to get monitor information for monitor at [%d, %d, %ux%u]", pos.X, pos.Y, size.Width, size.Height);
	}

	/* Append monitor to list. */
	availableDisplays.push_back(result);
	Log::Verbose("%s(%ux%u @%dHz) detected at [%d, %d], color depth: %u, gamma correction: %.2f.", result.name.c_str(), size.Width, size.Height, result.hertz, pos.X, pos.Y, result.depth, result.correction);
	return true;
}
#endif