#pragma once
#include "CodeChartIterator.h"

namespace Pu
{
	/* Defines a part of the unicode standart. */
	struct CodeChart
	{
	public:
		/* Initializes a new instance of a code chart with a specific range. */
		CodeChart(_In_ char32 start, _In_ char32 end);
		/* Copy constructor. */
		CodeChart(_In_ const CodeChart &value);
		/* Move constructor. */
		CodeChart(_In_ CodeChart &&value);

		/* Copy assignment. */
		_Check_return_ CodeChart& operator =(_In_ const CodeChart &other);
		/* Move assignment. */
		_Check_return_ CodeChart& operator =(_In_ CodeChart &&other);
		/* Appends a specific code chart to the this code chart. */
		_Check_return_ CodeChart operator +(_In_ const CodeChart &other) const;
		/* Appends a specific code chart to the this code chart. */
		_Check_return_ CodeChart& operator +=(_In_ const CodeChart &other);
		
		/* Gets the amount of characters in this code chart. */
		_Check_return_ size_t GetCharacterCount(void) const;
		/* Gets the iterator from the start of the range. */
		_Check_return_ CodeChartIterator begin(void);
		/* Gets the iterator at the end of the range. */
		_Check_return_ CodeChartIterator end(void);

		/* Basic Latin. */
		_Check_return_ static inline CodeChart ASCII(void)
		{
			return CodeChart(0x0000, 0x007F);
		}

		/* Katakana. */
		_Check_return_ static inline CodeChart Katakana(void)
		{
			return CodeChart(0x30A0, 0x30FF);
		}

		/* Miscellaneous Symbols. */
		_Check_return_ static inline CodeChart MiscSymbols(void)
		{
			return CodeChart(0x2600, 0x26FF);
		}


	private:
		using chart = std::pair<char32, char32>;
		vector<chart> ranges;

		CodeChart(size_t reserve);
	};
}