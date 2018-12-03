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
		/* Specifies the location (or handler) of this field. */
		uint32 Location;

		/* Initializes an invalid instance of the fieldinfo object. */
		FieldInfo(void)
			: Id(0), Name(), Type(FieldTypes::Invalid), Storage(spv::StorageClass::Max), Location(0)
		{}
		
		/* Initializes a new instance of the fieldinfo object. */
		FieldInfo(_In_ spv::Id id, _In_ string &&name, _In_ FieldTypes type, _In_ spv::StorageClass storage, _In_ uint32 location)
			: Id(id), Name(name), Type(type), Storage(storage), Location(location)
		{}

		/* Gets whether the field type is valid. */
		_Check_return_ inline bool IsValid(void) const
		{
			return Storage != spv::StorageClass::Max;
		}
	};
}