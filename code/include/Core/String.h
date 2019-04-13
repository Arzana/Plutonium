#pragma once
#include <algorithm>
#include <string>
#include <locale>
#include "Core/Math/Basics.h"
#include "Core/Collections/Vector.h"
#include "Core/Platform/Windows/Windows.h"
#include "Core/Diagnostics/NotImplementedException.h"

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
#pragma endregion
#pragma region converters
		/* Attempts to convert the string to UTF-8. */
		_Check_return_ inline basic_string<char> toUTF8(void) const
		{
			if constexpr (std::is_same<char_t, wchar_t>::value)
			{
#ifdef _WIN32
				return wideToMultiByte<char>(CP_UTF8);
#else
				throw NotImplementedException(typeid(toUTF8));
#endif
			}
			else throw NotImplementedException(typeid(toUTF8));
		}

		/* Attempts to convert the string to a wide string. */
		_Check_return_ inline basic_string<wchar_t> toWide(void) const
		{
			if constexpr (std::is_same<char_t, char>::value)
			{
#ifdef _WIN32
				return multiByteToWide<wchar_t>(CP_UTF8);
#else
				throw NotImplementedException(typeid(toWide));
#endif
			}
			else throw NotImplementedException(typeid(toWide));
		}

		/* Attempt to convert the string to a unicode string. */
		_Check_return_ inline basic_string<char32> toUTF32(void) const
		{
			if constexpr (std::is_same<char_t, char>::value)
			{
				return multiByteToUnicode();
			}
			else if constexpr (std::is_same<char_t, wchar_t>::value)
			{
#ifdef _WIN32
				return wideToUnicode();
#else
				return *this;
#endif
			}
			else return *this;
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

#ifdef _WIN32
		template <typename result_char_t>
		inline basic_string<result_char_t> multiByteToWide(UINT codePage) const
		{
			/* Create result buffer with enough size. */
			const size_type reserveSize = string_t::length() * sizeof(result_char_t);
			basic_string<result_char_t> result;
			result.resize(reserveSize);

			/* Convert using Win32 multi byte to wide char. */
			MultiByteToWideChar(codePage, 0, string_t::c_str(), -1, result.data(), static_cast<int>(reserveSize));
			return result;
		}

		template <typename result_char_t>
		inline basic_string<result_char_t> wideToMultiByte(UINT codePage) const
		{
			/* Create result buffer with enough size. */
			const size_type reserveSize = string_t::length() * sizeof(result_char_t);
			basic_string<result_char_t> result;
			result.resize(reserveSize);

			/* Convert using Win32 multi byte to multi byte. */
			WideCharToMultiByte(codePage, 0, string_t::c_str(), -1, result.data(), static_cast<int>(reserveSize), nullptr, nullptr);
			return result;
		}

		inline basic_string<char32> wideToUnicode() const
		{
			if constexpr (std::is_same<char_t, wchar_t>::value)
			{
				const size_type size = string_t::length();
				basic_string<char32> result;
				result.reserve(size);

				for (const wchar_t *start = string_t::data(), *end = start + size; start < end;)
				{
					const wchar_t uc = *start++;
					if ((uc - 0xD800u) >= 2048u) result += static_cast<char32>(uc);
					else
					{
						if ((uc & 0xFFFFFC00) == 0xD800 && start < end && (uc & 0xFFFFFC0) == 0xDC00)
						{
							result += (static_cast<char32>(uc) << 10) + static_cast<char32>(*start++) - 0x35FDC00;
						}
					}
				}

				return result;
			}
			else throw;
		}

		inline basic_string<char32> multiByteToUnicode() const
		{
			if constexpr (std::is_same<char_t, char>::value)
			{
				auto &facet = std::use_facet<std::codecvt<char32, char, std::mbstate_t>>(std::locale());
				basic_string<char32> result(string_t::length(), '\0');

				std::mbstate_t mb = std::mbstate_t();
				const char *from_next;
				char32 *to_next;

				facet.in(mb, 
					string_t::data(), string_t::data() + string_t::length(), from_next,
					result.data(), result.data() + string_t::length(), to_next);

				result.resize(to_next - result.data());
				return result;
			}
			else throw;
		}
#endif
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