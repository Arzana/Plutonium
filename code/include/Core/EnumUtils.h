#pragma once
#include <type_traits>

namespace Plutonium
{
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
}