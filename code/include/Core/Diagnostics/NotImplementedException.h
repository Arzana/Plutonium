#pragma once
#include <typeinfo>
#include <stdexcept>

namespace Pu
{
	/* Defines a specified exception for not implemented objects. */
	class NotImplementedException
		: public std::logic_error
	{
	public:
		/* Initializes a new instance of a not implemented exception. */
		NotImplementedException(_In_ const type_info &object)
			: logic_error(CreateMessage(object))
		{}

	private: 
		static std::string CreateMessage(const type_info &object)
		{
			std::string result = object.name();
			result += " is not yet implemented!";
			return result;
		}
	};
}