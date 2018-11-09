#pragma once
#include "NativeWindow.h"

namespace Pu
{
	class GameWindow
	{
	public:

	private:
		NativeWindow *native;
		bool allowUserResizing;
		bool isActive;
		bool isCursorVisible;
	};
}