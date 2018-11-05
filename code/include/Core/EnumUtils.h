#pragma once
#include <type_traits>

namespace Pu
{
#pragma region conversion
	/* Converts an enum to its underlying integer based type. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr typename std::underlying_type<_EnumTy>::type _CrtEnum2Int(_In_ _EnumTy e)
	{
		static_assert(std::is_enum<_EnumTy>::value, "Type is not a valid enum!");
		return static_cast<typename std::underlying_type<_EnumTy>::type>(e);
	}

	/* Converts an integer value to a deriving enum. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr _EnumTy _CrtInt2Enum(_In_ typename std::underlying_type<_EnumTy>::type i)
	{
		static_assert(std::is_enum<_EnumTy>::value, "Type is not a valid enum!");
		return static_cast<_EnumTy>(i);
	}
#pragma endregion
#pragma region bitwise operators
	/* Performs a bitwise or operation on the specified enum values. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr _EnumTy _CrtEnumBitOr(_In_ _EnumTy first, _In_ _EnumTy second)
	{
		return _CrtInt2Enum<_EnumTy>(_CrtEnum2Int(first) | _CrtEnum2Int(second));
	}

	/* Performs a bitwise and operation on the specified enum values. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr _EnumTy _CrtEnumBitAnd(_In_ _EnumTy first, _In_ _EnumTy second)
	{
		return _CrtInt2Enum<_EnumTy>(_CrtEnum2Int(first) & _CrtEnum2Int(second));
	}

	/* Performs a bitwise not operation on the specified enum value. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr _EnumTy _CrtEnumBitNot(_In_ _EnumTy e)
	{
		return _CrtInt2Enum<_EnumTy>(~_CrtEnum2Int(e));
	}
#pragma endregion
#pragma region utilities
	/* Returns whether the specified enum contains the specified flag. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr bool _CrtEnumCheckFlag(_In_ _EnumTy e, _In_ _EnumTy flag)
	{
		return _CrtEnumBitAnd(e, flag) == flag;
	}

	/* Removes a specified flag from the specified enum value. */
	template <typename _EnumTy>
	_Check_return_ inline constexpr _EnumTy _CrtEnumRemoveFlag(_In_ _EnumTy e, _In_ _EnumTy flag)
	{
		return _CrtEnumBitAnd(e, _CrtEnumBitNot(flag));
	}
#pragma endregion
}