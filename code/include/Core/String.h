#pragma once
#include <string>
#include <algorithm>
#include "Core/Math/Basics.h"
#include "Core/Collections/Vector.h"

namespace Pu
{
	/* Defines a wrapper object for ease of use string functions. */
	template<typename char_t, typename traits = std::char_traits<char_t>, typename allocator_t = typename std::basic_string<char_t>::allocator_type>
	class basic_string
		: public std::basic_string<char_t, traits, allocator_t>
	{
	public:
		using string_t = typename std::basic_string<char_t>;
		using size_type = typename string_t::size_type;

		/* Initializes an empty instance of a Plutonium string. */
		basic_string(void) noexcept(noexcept(allocator_t()))
			: string_t()
		{}

		/* Initializes an empty instance of aPlutonium string. */
		explicit basic_string(_In_ const allocator_t &alloc) noexcept
			: string_t(alloc)
		{}

		/* Initializes a new instance of a Plutonium string with a specified amount of characters initializes to a specified value. */
		basic_string(_In_ size_t count, _In_ char_t ch, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(count, ch, alloc)
		{}

		/* Initializes a new instance of a Plutonium string as a substring of another string. */
		basic_string(_In_ const basic_string<char_t> &other, _In_ size_t pos, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(other, pos, alloc)
		{}

		/* Initializes a new instance of a Plutonium string as a substring of another string. */
		basic_string(_In_ const basic_string<char_t> &other, _In_ size_t pos, _In_ size_t count, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(other, pos, count, alloc)
		{}

		/* Initializes a new instance of a Plutonium string as a substring of another string. */
		basic_string(_In_ const char_t *string, _In_ size_t count, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(string, count, alloc)
		{}

		/* Initializes a new instance of a Plutonium string as a copy of a null-terminated string. */
		basic_string(_In_ const char_t *string, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(string, alloc)
		{}

		/* Initialize a new instance of a Plutonium string with the contents of the range [first, last]. */
		template <class _ItTy>
		basic_string(_In_ _ItTy first, _In_ _ItTy last, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(first, last, alloc)
		{}

		/* Copy constructor. */
		basic_string(_In_ const basic_string<char_t> &other)
			: string_t(other)
		{}

		/* Copy constructor. */
		basic_string(_In_ const basic_string<char_t> &other, _In_ const allocator_t &alloc)
			: string_t(other, alloc)
		{}

		/* Move constructor. */
		basic_string(_In_ basic_string<char_t> &&other) noexcept
			: string_t(std::move(other))
		{}

		/* Move constructor. */
		basic_string(_In_ basic_string<char_t> &&other, _In_ const allocator_t &alloc) noexcept
			: string_t(std::move(other), alloc)
		{}

		/* Initializes a new instance of a Plutonium string from an initializer list. */
		basic_string(_In_ std::initializer_list<char_t> init, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(init, alloc)
		{}

		/* Implicitely converts the specified type to a string view. */
		template <class _Ty>
		basic_string(_In_ const _Ty &t, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(t, alloc)
		{}

		/* Implicitely converts the specified type to a string view. */
		template <class _Ty>
		basic_string(_In_ const _Ty &t, _In_ size_t pos, _In_ size_t n, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(t, pos, n, alloc)
		{}

		/* Replaces the contents with a copy of value. */
		_Check_return_ inline basic_string<char_t>& operator =(_In_ const basic_string<char_t> &other)
		{
			if (&other != this) string_t::operator=(other);
			return *this;
		}

		/* Replaces the contents with a copy of value. */
		_Check_return_ inline basic_string<char_t>& operator =(_In_ basic_string<char_t> &&other) noexcept
		{
			if (&other != this) string_t::operator=(std::move(other));
			return *this;
		}

		/* Replaces the contents with a copy of a null-terminated string. */
		_Check_return_ inline basic_string<char_t>& operator =(_In_ const char_t *string)
		{
			string_t::operator=(string);
			return *this;
		}

		/* Replaces the contents with a single character. */
		_Check_return_ inline basic_string<char_t>& operator =(_In_ char_t ch)
		{
			string_t::operator=(ch);
			return *this;
		}

		/* Replaces the contents with the contents of an initializer list. */
		_Check_return_ inline basic_string<char_t>& operator =(_In_ std::initializer_list<char_t> init)
		{
			string_t::operator=(init);
			return *this;
		}

		/* Implicitly converts the type to a string view. */
		template <class _Ty>
		_Check_return_ inline basic_string<char_t>& operator =(_In_ const _Ty &t)
		{
			string_t::operator=(t);
			return *this;
		}

		/* Checks if the input string is equal to the source string. */
		_Check_return_ inline bool operator ==(_In_ const basic_string<char_t> &other) const
		{
			return std::operator==(*this, other);
		}

		/* Checks if the input string is equal to the source string. */
		_Check_return_ inline bool operator ==(_In_ const char_t *other) const
		{
			return std::operator==(*this, other);
		}

		/* Checks if the input string differs from the source string. */
		_Check_return_ inline bool operator !=(_In_ const basic_string<char_t> &other) const
		{
			return std::operator!=(*this, other);
		}

		/* Checks if the input string differs from the source string. */
		_Check_return_ inline bool operator !=(_In_ const char_t *other) const
		{
			return std::operator!=(*this, other);
		}

		/* Appends string str. */
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ const basic_string<char_t> &str)
		{
			string_t::operator+=(str);
			return *this;
		}

		/* Appends a single character. */
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ char_t ch)
		{
			string_t::operator+=(ch);
			return *this;
		}

		/* Appends a null-terminates string. */
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ const char_t *str)
		{
			string_t::operator+=(str);
			return *this;
		}

		/* Appends the characters in the initializer list. */
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ std::initializer_list<char_t> init)
		{
			string_t::operator+=(init);
			return *this;
		}

		/* Implicitly converts the type of a string and appends it. */
		template <class _Ty>
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ _Ty &t)
		{
			string_t::operator+=(t);
			return *this;
		}

		/* Gets the amount of digits in a number if it would be converted to string. */
		_Check_return_ static inline size_t count_digits(_In_ uint64 number)
		{
			return number ? static_cast<size_t>(log10(number) + 1) : 1;
		}

		/* Gets the amount of digits in a number if it would be converted to string. */
		_Check_return_ static inline size_t count_digits(_In_ int64 number)
		{
			return number ? static_cast<size_t>(log10(abs(number)) + (number < 0 ? 2 : 1)) : 1;
		}

		/* Gets whether the string contains a specified substring. */
		_Check_return_ inline bool contains(_In_ basic_string<char_t> substr) const
		{
			return find_last_of(substr) != string_t::npos;
		}

		/* Removes a specified character from the string. */
		inline void remove(_In_ char_t ch)
		{
			string_t::erase(std::remove(string_t::begin(), string_t::end(), ch), string_t::end());
		}

		/* Removes the specified characters from the string. */
		inline void remove(_In_ std::initializer_list<char_t> init)
		{
			for (char_t ch : init) remove(ch);
		}

		/* Removes a spefic substring from the string. */
		inline void remove(_In_ basic_string<char_t> substr)
		{
			const size_type offset = string_t::find(substr);
			if (offset != string_t::npos) string_t::erase(offset, substr.length());
		}

		/* Removes the specified substrings from the string. */
		inline void remove(_In_ std::initializer_list<basic_string<char_t>> init)
		{
			for (basic_string<char_t> str : init) remove(str);
		}

		/* Gets a lower-case variant of the string. */
		_Check_return_ inline basic_string<char_t> toLower(void) const
		{
			return transformCopy([](char_t ch)
			{
				return static_cast<char_t>(tolower(static_cast<int>(ch)));
			});
		}

		/* Gets a upper-case variant of the string. */
		_Check_return_ inline basic_string<char_t> toUpper(void) const
		{
			return transformCopy([](char_t ch)
			{
				return static_cast<char_t>(toupper(static_cast<int>(ch)));
			});
		}

		/* Gets the last offset of the substring, starting from the specified position (if not npos). */
		_Check_return_ inline size_type find_last_of(_In_ const basic_string<char_t> &str, _In_opt_ size_type pos = string_t::npos) const noexcept
		{
			return string_t::find_last_of(str, pos);
		}

		/* Gets the last offset of the substring, starting from the specified position (if not npos). */
		_Check_return_ inline size_type find_last_of(_In_ const char_t *s, _In_opt_ size_type pos = string_t::npos) const
		{
			return string_t::find_last_of(s, pos);
		}

		/* Gets the last offset of the substring, starting from the specified position and up until the specified count. */
		_Check_return_ inline size_type find_last_of(_In_ const char_t *s, _In_ size_type pos, size_type n) const
		{
			return string_t::find_last_of(s, pos, n);
		}

		/* Gets the last offset of the char, starting from the specified position (if not npos). */
		_Check_return_ inline size_type find_last_of(_In_ char_t c, _In_opt_ size_type pos = string_t::npos) const noexcept
		{
			return string_t::find_last_of(c, pos);
		}

		/* Gets the last offset of any of the specified chars, starting from the specified position (if not npos). */
		_Check_return_ inline size_type find_last_of(_In_ std::initializer_list<char_t> init, _In_opt_ size_type pos = string_t::npos) const
		{
			size_type result = string_t::npos;

			for (const char_t c : init)
			{
				const size_type off = find_last_of(c, pos);
				if (off != string_t::npos) result = result != string_t::npos ? max(result, off) : off;
			}

			return result;
		}

		/* Splits the string into substrings using the specified seperator. */
		_Check_return_ inline vector<basic_string<char_t>> split(_In_ char_t seperator) const noexcept
		{
			vector<basic_string<char_t>> result;
			size_type prev = 0, pos = 0;

			while ((pos = string_t::find(seperator, pos)) != string_t::npos)
			{
				result.emplace_back(string_t::substr(prev, pos - prev));
				prev = ++pos;
			}

			const size_type len = string_t::length();
			if (len != prev) result.emplace_back(string_t::substr(prev, len - prev));
			return result;
		}

		/* Splits the string into substrings using the specified seperator. */
		_Check_return_ inline vector<basic_string<char_t>> split(_In_ const basic_string<char_t> &seperator) const noexcept
		{
			vector<basic_string<char_t>> result;
			size_type prev = 0, pos = 0;

			while ((pos = string_t::find(seperator, pos)) != string_t::npos)
			{
				result.emplace_back(string_t::substr(prev, pos - prev));
				prev = ++pos;
			}

			const size_type len = string_t::length();
			if (len != prev) result.emplace_back(string_t::substr(prev, len - prev));
			return result;
		}

	private:
		template <typename _FnTy>
		inline basic_string<char_t> transformCopy(const _FnTy func) const
		{
			basic_string<char_t> copy(*this);
			std::transform(string_t::begin(), string_t::end(), copy.begin(), func);
			return copy;
		}
	};

	/* Defines a string sorted in 8-bit characters. */
	using string = basic_string<char>;
	/* Defines a string stored in 32-bit characters. */
	using ustring = basic_string<char32_t>;
}

namespace std
{
	/* Add the Plutonium string as a hashable value. */
	template <>
	struct hash<Pu::string>
	{
	public:
		/* Defines the argument type for a hash. */
		using argument_type = Pu::string;
		/* Defines the result type for a hash. */
		using result_type = std::size_t;

		/* Calculates the hash from the specified argument. */
		_Check_return_ inline result_type operator ()(_In_ const argument_type &arg) const noexcept
		{
			return std::hash<std::string>{}(arg);
		}

		/* Calculates the hash from the specified arguments. */
		_Check_return_ inline result_type operator ()(_In_ const vector<argument_type> &args) const noexcept
		{
			argument_type str;
			for (const argument_type &cur : args) str += cur;
			return operator()(str);
		}

		/* Calculates the hash from the specified arguments. */
		_Check_return_ inline result_type operator ()(_In_ const std::initializer_list<argument_type> &args) const noexcept
		{
			argument_type str;
			for (const argument_type &cur : args) str += cur;
			return operator()(str);
		}
	};
}