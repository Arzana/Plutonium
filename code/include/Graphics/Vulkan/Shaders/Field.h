#pragma once
#include "Graphics/Vulkan/SPIR-V/FieldInfo.h"
#include "Core/Diagnostics/Logging.h"

namespace Pu
{
	/* Defines the base class for usable shader module fields. */
	class Field
	{
	public:
		/* The SPIR-V information of the field. */
		const FieldInfo &Info;

		/* Releases the resources allocated by the field. */
		virtual ~Field(void) {}

	protected:
		/* Initializes a new instance of a shader field. */
		Field(_In_ const FieldInfo &data)
			: Info(data)
		{}

		/* Checks if the specified type is accepted as input to the fields type. */
		_Check_return_ inline bool CheckInput(_In_ FieldType type) const
		{
			if (Info.Type != type)
			{
				Log::Error("Cannot set field %s %s with %s value!", Info.Type.GetName().c_str(), Info.Name.c_str(), type.GetName().c_str());
				return false;
			}

			return true;
		}
	};
}