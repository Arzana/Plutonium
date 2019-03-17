#pragma once
#include "Core/Math/Constants.h"
#include "Core/Platform/Windows/Windows.h"

namespace Pu
{
	/* Defines some common keys for easy default key bindings. */
	enum class Keys
		: uint16
	{
#ifdef _WIN32
#pragma region Unknow & space
		Unknown = 0,
		Space = VK_SPACE,
#pragma endregion
#pragma region Numbers
		D0 = '0',
		D1 = '1',
		D2 = '2',
		D3 = '3',
		D4 = '4',
		D5 = '5',
		D6 = '6',
		D7 = '7',
		D8 = '8',
		D9 = '9',
#pragma endregion
#pragma region Characters
		A = 'A',
		B = 'B',
		C = 'C',
		D = 'D',
		E = 'E',
		F = 'F',
		G = 'G',
		H = 'H',
		I = 'I',
		J = 'J',
		K = 'K',
		L = 'L',
		M = 'M',
		N = 'N',
		O = 'O',
		P = 'P',
		Q = 'Q',
		R = 'R',
		S = 'S',
		T = 'T',
		U = 'U',
		V = 'V',
		W = 'W',
		X = 'X',
		Y = 'Y',
		Z = 'Z',
#pragma endregion
#pragma region Special characters (OEM specific)
		OemApostrophe = VK_OEM_7,
		OemComma = VK_OEM_COMMA,
		OemMinus = VK_OEM_MINUS,
		OemPeriod = VK_OEM_PERIOD,
		OemSlash = VK_OEM_2,
		OemSemiColon = VK_OEM_1,
		OemOpenBracket = VK_OEM_4,
		OemBlackslash = VK_OEM_5,
		OemCloseBracket = VK_OEM_6,
		OemTilde = VK_OEM_3,
#pragma endregion
#pragma region Functional
		Escape = VK_ESCAPE,
		Enter = VK_RETURN,
		Tab = VK_TAB,
		Backspace = VK_BACK,
		Insert = VK_INSERT,
		Delete = VK_DELETE,
		Right = VK_RIGHT,
		Left = VK_LEFT,
		Down = VK_DOWN,
		Up = VK_UP,
		PageUp = VK_PRIOR,
		PageDown = VK_NEXT,
		Home = VK_HOME,
		End = VK_END,
		CapsLock = VK_CAPITAL,
		ScrollLock = VK_SCROLL,
		NumLock = VK_NUMLOCK,
		PrintScreen = VK_SNAPSHOT,
		Pause = VK_PAUSE,
#pragma endregion
#pragma region F keys
		F1 = VK_F1,
		F2 = VK_F2,
		F3 = VK_F3,
		F4 = VK_F4,
		F5 = VK_F5,
		F6 = VK_F6,
		F7 = VK_F7,
		F8 = VK_F8,
		F9 = VK_F9,
		F10 = VK_F10,
		F11 = VK_F11,
		F12 = VK_F12,
		F13 = VK_F13,
		F14 = VK_F14,
		F15 = VK_F15,
		F16 = VK_F16,
		F17 = VK_F17,
		F18 = VK_F18,
		F19 = VK_F19,
		F20 = VK_F20,
		F21 = VK_F21,
		F22 = VK_F22,
		F23 = VK_F23,
		F24 = VK_F24,
#pragma endregion
#pragma region Numpad
		N0 = VK_NUMPAD0,
		N1 = VK_NUMPAD1,
		N2 = VK_NUMPAD2,
		N3 = VK_NUMPAD3,
		N4 = VK_NUMPAD4,
		N5 = VK_NUMPAD5,
		N6 = VK_NUMPAD6,
		N7 = VK_NUMPAD7,
		N8 = VK_NUMPAD8,
		N9 = VK_NUMPAD9,
		NumDecimal = VK_DECIMAL,
		NumDivide = VK_DIVIDE,
		NumMultiply = VK_MULTIPLY,
		NumSubtract = VK_SUBTRACT,
		NumAdd = VK_ADD,
#pragma endregion
#pragma region Modifiers
		LeftShift = VK_LSHIFT,
		LeftControl = VK_LCONTROL,
		Alt = VK_MENU,
		LeftWindows = VK_LWIN,
		RightShift = VK_RSHIFT,
		RightControl = VK_RCONTROL,
		RightWindows = VK_RWIN,
		Menu = VK_MENU
#pragma endregion
#endif
	};
}