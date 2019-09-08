#pragma once
#include <algorithm>
#include <string>
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

#pragma region ctors
		/* Initializes an empty instance of a Plutonium string. */
		basic_string(void) noexcept(noexcept(allocator_t()))
			: string_t()
		{}

		/* Initializes an empty instance of aPlutonium string. */
		explicit basic_string(_In_ const allocator_t &alloc) noexcept
			: string_t(alloc)
		{}

		/* Initializes a new instance of a Plutonium string with a specific amount of uninitialized characters. */
		basic_string(_In_ size_t count, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(alloc)
		{
			string_t::reserve(count);
			string_t::_Get_data()._Mysize = count;
		}

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
		template <class other_char_t>
		explicit basic_string(_In_ const other_char_t &t, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(t, alloc)
		{}

		/* Implicitely converts the specified type to a string view. */
		template <class _Ty>
		basic_string(_In_ const _Ty &t, _In_ size_t pos, _In_ size_t n, _In_opt_ const allocator_t &alloc = allocator_t())
			: string_t(t, pos, n, alloc)
		{}

		/* Initializes a new instance of a Plutonium string from a std string. */
		basic_string(_In_ const std::basic_string<char_t> &stdString)
			: string_t(stdString)
		{}
#pragma endregion
#pragma region operators
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
		_Check_return_ inline basic_string<char_t> operator +(_In_ const basic_string<char_t> &str) const
		{
			return std::operator+(*this, str);
		}

		/* Appends a single character. */
		_Check_return_ inline basic_string<char_t> operator +(_In_ char_t ch) const
		{
			return std::operator+(*this, ch);
		}

		/* Appends a null-terminates string. */
		_Check_return_ inline basic_string<char_t> operator +(_In_ const char_t *str) const
		{
			return std::operator+(*this, str);
		}

		/* Appends the characters in the initializer list. */
		_Check_return_ inline basic_string<char_t> operator +(_In_ std::initializer_list<char_t> init) const
		{
			return std::operator+(*this, init);
		}

		/* Implicitly converts the type of a string and appends it. */
		template <class other_character_t>
		_Check_return_ inline basic_string<char_t> operator +(_In_ other_character_t &t) const
		{
			return std::operator+(*this, t);
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
		template <class other_character_t>
		_Check_return_ inline basic_string<char_t>& operator +=(_In_ other_character_t &t)
		{
			string_t::operator+=(t);
			return *this;
		}
#pragma endregion
#pragma region utility
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

		/* Gets the file extension from the string, or an empty string if the string doesn't contain one (dot is not included). */
		_Check_return_ basic_string<char_t> fileExtension(void) const
		{
			const size_type offset = string_t::find_last_of(static_cast<char_t>(U'.'));
			return offset != string_t::npos ? string_t::substr(offset + 1, string_t::length() - offset - 1) : basic_string();
		}

		/* Gets the file name from the string, or the entire string if the string wasn't recognized as a path. */
		_Check_return_ basic_string<char_t> fileName(void) const
		{
			const size_type start = find_last_of({ static_cast<char_t>(U'/'), static_cast<char_t>(U'\\') }) + 1;
			return string_t::substr(start, string_t::length() - start);
		}

		/* Gets the file name without extension from the string, or the entire string if the string wasn't recognized as a path. */
		_Check_return_ basic_string<char_t> fileNameWithoutExtension(void) const
		{
			const size_type start = find_last_of({ static_cast<char_t>(U'/'), static_cast<char_t>(U'\\') }) + 1;
			size_type end = find_last_of(static_cast<char_t>(U'.'));
			if (end == string_t::npos) end = string_t::length();
			return string_t::substr(start, end - start);
		}

		/* Gets the file directory from the string, or an empty string if no path was present. */
		_Check_return_ basic_string<char_t> fileDirectory(void) const
		{
			const size_type len = find_last_of({ static_cast<char_t>(U'/'), static_cast<char_t>(U'\\') });
			return len != string_t::npos ? string_t::substr(0, len + 1) : basic_string<char_t>();
		}

		/* Gets the file directory and file name from the string, or an empty string if it could not be calculated. */
		_Check_return_ basic_string<char_t> fileWithoutExtension(void) const
		{
			const size_type len = find_last_of(static_cast<char_t>(U'.'));
			return len != string_t::npos ? string_t::substr(0, len) : basic_string<char_t>();
		}
#pragma endregion
#pragma region queries
		/* Gets whether the string contains a specified substring. */
		_Check_return_ inline bool contains(_In_ basic_string<char_t> substr) const
		{
			return string_t::find(substr) != string_t::npos;
		}

		/* Gets whether the string contains a specified substring. */
		_Check_return_ inline bool contains(_In_ const char_t *substr) const
		{
			return string_t::find(substr) != string_t::npos;
		}

		/* Gets the occurance of the specified character in the string. */
		_Check_return_ inline size_t count(_In_ char_t ch) const
		{
			return std::count(string_t::begin(), string_t::end(), ch);
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
			return transformCopy<char_t>([](char_t ch)
			{
				return static_cast<char_t>(tolower(static_cast<int>(ch)));
			});
		}

		/* Gets a upper-case variant of the string. */
		_Check_return_ inline basic_string<char_t> toUpper(void) const
		{
			return transformCopy<char_t>([](char_t ch)
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

		/* Replaces a specific substring in this string with another string. */
		_Check_return_ inline void replace(_In_ const basic_string<char_t> &search, _In_ const basic_string<char_t> &replace)
		{
			for (size_type start = 0, len = search.length(); (start = string_t::find(search, start)) != string_t::npos; start += len)
			{
				string_t::replace(start, len, replace);
			}
		}

		/* Replaces a specific substring in this string with another string, returns whether it was replaced. */
		_Check_return_ inline bool tryReplace(_In_ const basic_string<char_t> &search, _In_ const basic_string<char_t> &replace)
		{
			bool result = false;
			size_type start = 0;
			const size_type len = search.length();

			while ((start = string_t::find(search, start)) != string_t::npos)
			{
				string_t::replace(start, len, replace);
				start += len;
				result = true;
			}

			return result;
		}

		/* Splits the string into substrings using the specified seperators. */
		_Check_return_ inline vector<basic_string<char_t>> split(_In_ std::initializer_list<char_t> seperators) const noexcept
		{
			vector<basic_string<char_t>> result;

			bool first = true;
			for (char_t c : seperators)
			{
				if (first)
				{
					result = split(c);
					first = false;
				}
				else
				{
					vector<basic_string<char_t>> buffer;

					for (const basic_string<char_t> str : result)
					{
						const vector<basic_string<char_t>> tmp = str.split(c);
						buffer.insert(buffer.end(), tmp.begin(), tmp.end());
					}

					result = std::move(buffer);
				}
			}

			return result;
		}

		/* Splits the string into substrings using the specified seperator. */
		_Check_return_ inline vector<basic_string<char_t>> split(_In_ char_t seperator) const noexcept
		{
			vector<basic_string<char_t>> result;
			size_type prev = 0, pos = 0, len;

			while ((pos = string_t::find(seperator, pos)) != string_t::npos)
			{
				len = pos - prev;
				if (len) result.emplace_back(string_t::substr(prev, len));
				prev = ++pos;
			}

			len = string_t::length();
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

		/* Gets a specific substring from within this string. */
		_Check_return_ inline basic_string<char_t> substr(_In_ size_type pos = 0, _In_ size_type count = string_t::npos) const
		{
			return string_t::substr(pos, count);
		}

		/* Removes leading and trailing character from the string. */
		_Check_return_ inline basic_string<char_t> trim(_In_ const char_t *characters) const
		{
			const size_type begin = string_t::find_first_not_of(characters);
			if (begin == string_t::npos) return basic_string<char_t>();

			const size_type end = string_t::find_last_not_of(characters);
			if (end == string_t::npos) return string_t::substr(begin);
			return string_t::substr(begin, end - begin + 1);
		}

		/* Removes leading characters from the string. */
		_Check_return_ inline basic_string<char_t> trim_front(_In_ const char_t *characters) const
		{
			const size_type begin = string_t::find_first_not_of(characters);
			if (begin == string_t::npos) return basic_string<char_t>();
			return string_t::substr(begin);
		}

		/* Remobes trailing characters from the string. */
		_Check_return_ inline basic_string<char_t> trim_back(_In_ const char_t *characters) const
		{
			const size_type end = string_t::find_last_not_of(characters);
			if (end == string_t::npos) return basic_string<char_t>();
			return string_t::substr(0, end + 1);
		}
#pragma endregion
#pragma region converters
		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ int32 value)
		{
			return to_string(value);
		}

		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ int64 value)
		{
			return to_string(value);
		}

		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ uint32 value)
		{
			return to_string(value);
		}

		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ uint64 value)
		{
			return to_string(value);
		}

		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ float value)
		{
			return to_string(value);
		}

		/* Converts the integer value to a string. */
		_Check_return_ static inline basic_string<char_t> from(_In_ double value)
		{
			return to_string(value);
		}

		/* Attempts to convert the string to UTF-8. */
		_Check_return_ inline basic_string<char> toUTF8(void) const
		{
			converter<basic_string<char>> conv;
			return conv(*this);
		}

		/* Attempts to convert the string to a wide string. */
		_Check_return_ inline basic_string<wchar_t> toWide(void) const
		{
			converter<basic_string<wchar_t>> conv;
			return conv(*this);
		}

		/* Attempt to convert the string to a unicode string. */
		_Check_return_ inline basic_string<char32> toUTF32(void) const
		{
			converter<basic_string<char32>> conv;
			return conv(*this);
		}
#pragma endregion

	private:
		template <typename result_char_t, typename lambda_t>
		inline basic_string<result_char_t> transformCopy(const lambda_t func) const
		{
			basic_string<result_char_t> copy(*this);
			std::transform(string_t::begin(), string_t::end(), copy.begin(), func);
			return copy;
		}

		template <typename value_t>
		static inline basic_string<char_t> to_string(value_t value)
		{
			basic_string<char> result = std::to_string(value);
			if constexpr (std::is_same_v<char_t, char>) return result;
			else if constexpr (std::is_same_v<char_t, wchar_t>) return result.toWide();
			else if constexpr (std::is_same_v<char_t, char32>) return result.toUTF32();
			else static_assert(true, "Cannot convert from UTF-8 string to specified string!");
		}

		template <typename result_t>
		struct converter
		{
			template <typename T, typename = void>
			struct is_range
				: std::false_type
			{};

			template <typename T>
			struct is_range<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> final
				: std::true_type
			{};

			template <typename in_t>
			result_t operator ()(in_t arg)
			{
				if constexpr (is_range<in_t>::value) return { arg.begin(), arg.end() };
				else return this->operator()(basic_string<std::remove_cv_t<std::remove_pointer<in_t>>>(arg));
			}
		};
	};

	/* Defines a string sorted in 8-bit characters. */
	using string = basic_string<char>;
	/* Defines a string stored in either 16-bit characters (Windows) or 32-bit characters. */
	using wstring = basic_string<wchar_t>;
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
			return std::hash<std::string>{}(str);
		}

		/* Calculates the hash from the specified arguments. */
		_Check_return_ inline result_type operator ()(_In_ const std::initializer_list<argument_type> &args) const noexcept
		{
			argument_type str;
			for (const argument_type &cur : args) str += cur;
			return std::hash<std::string>{}(str);
		}
	};

	/* Add the Plutonium wide string as a hashable value. */
	template <>
	struct hash<Pu::wstring>
	{
	public:
		/* Defines the argument type for a hash. */
		using argument_type = Pu::wstring;
		/* Defines the result type for a hash. */
		using result_type = std::size_t;

		/* Calculates the hash from the specified argument. */
		_Check_return_ inline result_type operator ()(_In_ const argument_type &arg) const noexcept
		{
			return std::hash<std::wstring>{}(arg);
		}

		/* Calculates the hash from the specified arguments. */
		_Check_return_ inline result_type operator ()(_In_ const vector<argument_type> &args) const noexcept
		{
			argument_type str;
			for (const argument_type &cur : args) str += cur;
			return std::hash<std::wstring>{}(str);
		}

		/* Calculates the hash from the specified arguments. */
		_Check_return_ inline result_type operator ()(_In_ const std::initializer_list<argument_type> &args) const noexcept
		{
			argument_type str;
			for (const argument_type &cur : args) str += cur;
			return std::hash<std::wstring>{}(str);
		}
	};
}