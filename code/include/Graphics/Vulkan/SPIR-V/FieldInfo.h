#pragma once
#include "Decoration.h"
#include "FieldType.h"

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
		FieldType Type;
		/* Specifies the amount of array elements in this field (zero indicates no array). */
		uint32 ArrayElements;
		/* Specifies how the type is being used by the shader. */
		spv::StorageClass Storage;
		/* Specifies the decorations applied to this field. */
		Decoration Decorations;

		/* Initializes an invalid instance of the fieldinfo object. */
		FieldInfo(void)
			: Id(0), Name(), Type(), ArrayElements(0), Storage(spv::StorageClass::Max)
		{}
		
		/* Initializes a new instance of the fieldinfo object. */
		FieldInfo(_In_ spv::Id id, _In_ string &&name, _In_ const FieldType &type, _In_ spv::StorageClass storage, _In_ const Decoration &decorations)
			: Id(id), Name(name), Type(type), ArrayElements(0), Storage(storage), Decorations(decorations)
		{}

		/* Copy contructor. */
		FieldInfo(_In_ const FieldInfo&) = default;
		/* Move constructor. */
		FieldInfo(_In_ FieldInfo&&) = default;

		/* Copy assignment. */
		_Check_return_ FieldInfo& operator =(_In_ const FieldInfo&) = default;
		/* Move assignment. */
		_Check_return_ FieldInfo& operator =(_In_ FieldInfo&&) = default;

		/* Gets whether the field type is valid. */
		_Check_return_ inline bool IsValid(void) const
		{
			return Storage != spv::StorageClass::Max;
		}

		/* Gets the location decoration literal. */
		_Check_return_ inline spv::Word GetLocation(void) const
		{
			std::map<spv::Decoration, spv::Word>::const_iterator it = Decorations.Numbers.find(spv::Decoration::Location);
			if (it == Decorations.Numbers.end()) return 0;
			
			return it->second;
		}
	};
}