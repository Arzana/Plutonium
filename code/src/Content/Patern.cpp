#include "Content/Patern.h"

Pu::Patern::Patern(const Patern & value)
	: wildcards(value.wildcards)
{}

Pu::Patern::Patern(Patern && value)
	: wildcards(std::move(value.wildcards))
{}

Pu::Patern & Pu::Patern::operator=(const Patern & other)
{
	if (this != &other)
	{
		wildcards = other.wildcards;
	}

	return *this;
}

Pu::Patern & Pu::Patern::operator=(Patern && other)
{
	if (this != &other)
	{
		wildcards = std::move(other.wildcards);
	}

	return *this;
}

void Pu::Patern::AddWildcard(const wstring & patern, const wstring & definition)
{
	/* Replace the previous wilcard if it already exists in the map, otherwise; just add it. */
	Dictionary::iterator it = wildcards.find(patern);
	if (it != wildcards.end()) it->second = definition;
	else wildcards.emplace(patern, definition);
}

void Pu::Patern::Solve(wstring & str) const
{
	/* Recusively solve for all the wildcards. */
	for (const auto&[patern, definition] : wildcards)
	{
		/* We only need to go into a recursion if a solve occured, otherwise we'd get an endless loop. */
		if (str.tryReplace(patern, definition)) Solve(str);
	}
}

Pu::vector<Pu::wstring> Pu::Patern::Solve(std::initializer_list<wstring> strings) const
{
	vector<wstring> result(strings);

	for (wstring &cur : result)
	{
		Solve(cur);
	}

	return result;
}