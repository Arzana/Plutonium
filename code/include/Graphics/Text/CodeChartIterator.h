#pragma once
#include "Core/Math/Constants.h"
#include "Core/Collections/vector.h"

namespace Pu
{
	/* Defines an iterator that chan iterate over all elements in a code chart list. */
	class CodeChartIterator
	{
	public:
		/* Defines a random access iterator. */
		using iterator_category = std::random_access_iterator_tag;
		/* UTF-32 character. */
		using value_type = char32;
		/* UTF-32 character. */
		using difference_type = value_type;
		/* UTF-32 pointer. */
		using pointer = value_type * ;
		/* UTF-32 reference. */
		using reference = value_type & ;

		/* Copy constructor. */
		CodeChartIterator(_In_ const CodeChartIterator &value)
			: item(value.item), isEnd(value.isEnd), incrementable(value.incrementable)
		{
			if (isEnd) InitAsEnd();
			else
			{
				values = value.values;
				it = value.it;
			}
		}

		/* Move constructor. */
		CodeChartIterator(_In_ CodeChartIterator &&value)
			: values(value.values), it(std::move(value.it)), item(value.item), 
			isEnd(value.isEnd), incrementable(value.incrementable)
		{
			if (isEnd)
			{
				value.isEnd = false;
				value.values = nullptr;
			}
		}

		/* Releases the resources allocated by the iterator. */
		~CodeChartIterator(void)
		{
			if (isEnd) delete values;
		}

		_Check_return_ inline CodeChartIterator& operator =(_In_ const CodeChartIterator &) = delete;
		_Check_return_ inline CodeChartIterator& operator =(_In_ CodeChartIterator &&) = delete;

		/* Precrement. */
		_Check_return_ inline CodeChartIterator& operator ++(void)
		{
			if (incrementable)
			{
				if (++item > it->second)
				{
					if (++it != values->end()) item = it->first;
					else incrementable = false;
				}
			}

			return *this;
		}

		/* Postcrement. */
		_Check_return_ inline CodeChartIterator operator ++(int)
		{
			CodeChartIterator result(*this);
			return ++result;
		}

		/* Checks if the two iterators are at the same position. */
		_Check_return_ inline bool operator ==(_In_ const CodeChartIterator &other) const
		{
			return other.item == item;
		}

		/* Checks if the two iterators are not at the same position. */
		_Check_return_ inline bool operator !=(_In_ const CodeChartIterator &other) const
		{
			return other.item != item;
		}

		/* Gets the current item from the iterator. */
		_Check_return_ inline value_type operator *(void) const
		{
			return item;
		}

	private:
		friend struct CodeChart;

		const vector<std::pair<char32, char32>> *values;
		vector<std::pair<char32, char32>>::const_iterator it;
		char32 item;
		bool isEnd, incrementable;

		CodeChartIterator(const vector<std::pair<char32, char32>> &list)
			: values(&list), it(list.begin()), item(it->first), 
			isEnd(false), incrementable(true)
		{}

		CodeChartIterator(char32 end)
			: item(end), isEnd(true), incrementable(true)
		{
			InitAsEnd();
		}

		void InitAsEnd(void)
		{
			values = new vector<std::pair<char32, char32>>(1, std::make_pair(item, item));
			it = values->begin();
		}
	};
}