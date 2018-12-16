#pragma once
#include "Core/String.h"
#include "Core/Platform/Windows/Windows.h"
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Core/Math/Vector2.h"

namespace Pu
{
	/* Defines visible information about physical displays. */
	class Display
	{
	public:
		/* Copy constructor. */
		Display(_In_ const Display &value);
		/* Move constructor. */
		Display(_In_ Display &&value);

		/* Copy assignment. */
		_Check_return_ Display& operator =(_In_ const Display &other);
		/* Move assignment. */
		_Check_return_ Display& operator =(_In_ Display &&other);

		/* Gets the assigned name of the display. */
		_Check_return_ const string& GetName(void) const
		{
			return name;
		}

		/* Gets the rectangle that specified the usable bounds of the display. */
		_Check_return_ inline Rect2D GetClientBounds(void) const
		{
			return viewport;
		}

		/* Gets the position of the display as a vector. */
		_Check_return_ inline Vector2 GetPosition(void) const
		{
			return Vector2(static_cast<float>(viewport.Offset.X), static_cast<float>(viewport.Offset.Y));
		}

		/* Gets the size of the display as a vector. */
		_Check_return_ inline Vector2 GetSize(void) const
		{
			return Vector2(static_cast<float>(viewport.Extent.Width), static_cast<float>(viewport.Extent.Height));
		}

		/* Gets the refresh rate of the display in hertz. */
		_Check_return_ inline uint32 GetRefreshRate(void) const
		{
			return hertz;
		}

		/* Gets the amount of bits available per color channel. */
		_Check_return_ inline uint32 GetColorDepth(void) const
		{
			return depth;
		}

		/* Gets the average gamma correction that should be applied to the final pixel color. */
		_Check_return_ inline float GetGammaCorrection(void) const
		{
			return correction;
		}

		/* Gets the display specified as the primary display by the operating system. */
		_Check_return_ static const Display& GetPrimaryDisplay(void);
		/* Gets the display at the specified point in view space. */
		_Check_return_ static const Display& GetDisplayAt(_In_ Offset2D point);
		/* Gets all displays visible to the operating system. */
		_Check_return_ static const vector<Display>& GetAll(void);

	private:
		string name;
		Rect2D viewport;
		uint32 hertz, depth;
		bool isPrimary;
		float correction;

		Display(void);

		static void FindDisplays(void);

#ifdef _WIN32
		static BOOL MonitorProc(HMONITOR monitor, HDC, LPRECT vp, LPARAM);
#endif
	};
}