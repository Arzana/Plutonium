#pragma once
#include "SPIRV.h"
#include "FieldTypes.h"
#include "Core/String.h"

namespace Pu
{
	/* Defines the raw information about a SPIR-V field. */
	struct FieldInfo
	{
	public:
		/* Specifies the unique indentifier of the field. */
		spv::Id Id;
		/* Specifies the name of the field. */
		string Name;
		/* Specifies the data type of the field. */
		FieldTypes Type;
		/* Specifies how the type is being used by the shader. */
		spv::StorageClass Storage;

		/* Initializes an invalid instance of the fieldinfo object. */
		FieldInfo(void)
			: Id(0), Name(), Type(FieldTypes::Invalid), Storage(spv::StorageClass::Max)
		{}
		
		/* Initializes a new instance of the fieldinfo object. */
		FieldInfo(spv::Id id, string &&name, FieldTypes type, spv::StorageClass storage)
			: Id(id), Name(name), Type(type), Storage(storage)
		{}

		/* Gets whether this instance is a valid instance. */
		_Check_return_ inline bool IsValid(void) const
		{
			return !Name.empty() && Type != FieldTypes::Invalid;
		}
	};
}