#pragma once
#include <type_traits>

namespace Pu
{
#pragma region conversion
	/* Converts an enum to its underlying integer based type. */
	template <typename enum_t>
	_Check_return_ inline constexpr typename std::underlying_type<enum_t>::type _CrtEnum2Int(_In_ enum_t e)
	{
		static_assert(std::is_enum<enum_t>::value, "Type is not a valid enum!");
		return static_cast<typename std::underlying_type<enum_t>::type>(e);
	}

	/* Converts an integer value to a deriving enum. */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtInt2Enum(_In_ typename std::underlying_type<enum_t>::type i)
	{
		static_assert(std::is_enum<enum_t>::value, "Type is not a valid enum!");
		return static_cast<enum_t>(i);
	}
#pragma endregion
#pragma region bitwise operators
	/* Performs a bitwise or operation on the specified enum values. */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtEnumBitOr(_In_ enum_t first, _In_ enum_t second)
	{
		return _CrtInt2Enum<enum_t>(_CrtEnum2Int(first) | _CrtEnum2Int(second));
	}

	/* Performs a bitwise and operation on the specified enum values. */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtEnumBitAnd(_In_ enum_t first, _In_ enum_t second)
	{
		return _CrtInt2Enum<enum_t>(_CrtEnum2Int(first) & _CrtEnum2Int(second));
	}

	/* Performs a bitwise not operation on the specified enum value. */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtEnumBitNot(_In_ enum_t e)
	{
		return _CrtInt2Enum<enum_t>(~_CrtEnum2Int(e));
	}
#pragma endregion
#pragma region bitwise assignment operators
	/* Performs a bitwise or operation on the specified enum values and stores the result in the first value. */
	template <typename enum_t>
	_Check_return_ inline enum_t _CrtEnumBitOrSet(_In_ enum_t &first, _In_ enum_t second)
	{
		return first = _CrtEnumBitOr(first, second);
	}

	/* Performs a bitwise and operation on the specified enum values and stores the result in the first value. */
	template <typename enum_t>
	_Check_return_ inline enum_t _CrtEnumBitAndSet(_In_ enum_t &first, _In_ enum_t second)
	{
		return first = _CrtEnumBitAnd(first, second);
	}

	/* Performs a bitwise not operation on the specified enum values and stores the result in the first value. */
	template <typename enum_t>
	_Check_return_ inline enum_t _CrtEnumBitNotSet(_In_ enum_t &first, _In_ enum_t second)
	{
		return first = _CrtEnumBitNot(first, second);
	}
#pragma endregion
#pragma region utilities
	/* Returns whether the specified enum contains the specified flag. */
	template <typename enum_t>
	_Check_return_ inline constexpr bool _CrtEnumCheckFlag(_In_ enum_t e, _In_ enum_t flag)
	{
		return _CrtEnumBitAnd(e, flag) == flag;
	}

	/* Adds a specific flag from the specified enum value (just a wrapper around _CrtEnumBitOr). */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtEnumAddFlag(_In_ enum_t e, _In_ enum_t flag)
	{
		return _CrtEnumBitOr(e, flag);
	}

	/* Removes a specific flag from the specified enum value. */
	template <typename enum_t>
	_Check_return_ inline constexpr enum_t _CrtEnumRemoveFlag(_In_ enum_t e, _In_ enum_t flag)
	{
		return _CrtEnumBitAnd(e, _CrtEnumBitNot(flag));
	}
#pragma endregion
}