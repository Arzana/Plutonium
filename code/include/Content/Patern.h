#pragma once
#include <map>
#include "Core/String.h"

namespace Pu
{
	/* Defines an object that can convert from paterns to full strings. */
	class Patern
	{
	public:
		/* Initializes a new instance of a patern. */
		Patern(void) = default;
		/* Copy constructor. */
		Patern(_In_ const Patern &value);
		/* Move constructor. */
		Patern(_In_ Patern &&value);

		/* Copy assignment. */
		_Check_return_ Patern& operator =(_In_ const Patern &other);
		/* Move assignment. */
		_Check_return_ Patern& operator =(_In_ Patern &&other);

		/* Adds a wildcard to this patern. */
		void AddWildcard(_In_ const wstring &patern, _In_ const wstring &definition);
		/* Substitudes all paterns known in the string for their definitions. */
		void Solve(_In_ wstring &str) const;
		/* Substitudes all paterns known in the strings for their definitions. */
		_Check_return_ vector<wstring> Solve(_In_ std::initializer_list<wstring> strings) const;

	private:
		using Dictionary = std::map<wstring, wstring>;

		Dictionary wildcards;
	};
}