#include "Graphics/Text/CodeChart.h"
#include "Core/Diagnostics/Logging.h"

Pu::CodeChart::CodeChart(size_t reserve)
{
	ranges.reserve(reserve);
}

Pu::CodeChart::CodeChart(char32 start, char32 end)
{
	if (start >= end) Log::Fatal("Invalid range passed to code chart!");
	ranges.emplace_back(std::make_pair(start, end));

	/* Default include the special range to allow for replacement character. */
	ranges.emplace_back(std::make_pair(0xFFF0, 0xFFFF));
}

Pu::CodeChart::CodeChart(const CodeChart & value)
	: ranges(value.ranges)
{}

Pu::CodeChart::CodeChart(CodeChart && value)
	: ranges(std::move(value.ranges))
{}

Pu::CodeChart & Pu::CodeChart::operator=(const CodeChart & other)
{
	if (this != &other) ranges = other.ranges;
	return *this;
}

Pu::CodeChart & Pu::CodeChart::operator=(CodeChart && other)
{
	if (this != &other) ranges = std::move(other.ranges);
	return *this;
}

Pu::CodeChart Pu::CodeChart::operator+(const CodeChart & other) const
{
	CodeChart result(*this);
	return result += other;
}

Pu::CodeChart & Pu::CodeChart::operator+=(const CodeChart & other)
{
	vector<chart>::iterator it = ranges.begin();
	for (vector<chart>::const_iterator oit = other.ranges.begin(); oit != other.ranges.end(); oit++)
	{
		/* Move forward until we found our starting point. */
		while (it != ranges.end() && it->second < oit->first) it++;

		if (it == ranges.end())
		{
			/* If we've reached the end we can just append the remaining code charts to the result and early out.  */
			for (; oit != other.ranges.end(); oit++) ranges.emplace_back(*oit);
			break;
		}
		else if (it->first > oit->first && oit->second < it->first)
		{
			/* The other range is fully before this range so just add it. */
			ranges.emplace(it, *oit);
		}
		else
		{
			/* Alter the last range in this list to include the current range. */
			it->first = min(it->first, oit->first);
			it->second = max(it->second, oit->second);
		}
	}

	return *this;
}

size_t Pu::CodeChart::GetCharacterCount(void) const
{
	size_t result = 0;
	for (const auto[start, end] : ranges) result += end - start;
	return result;
}

Pu::CodeChartIterator Pu::CodeChart::begin(void)
{
	return CodeChartIterator(ranges);
}

Pu::CodeChartIterator Pu::CodeChart::end(void)
{
	return CodeChartIterator(ranges[ranges.size() -1].second + 1);
}
