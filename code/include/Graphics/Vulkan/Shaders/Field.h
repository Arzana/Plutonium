#pragma once
#include "Graphics/Vulkan/SPIR-V/FieldInfo.h"
#include "Core/Diagnostics/Logging.h"

namespace Pu
{
	/* Defines the base class for usable shader module fields. */
	class Field
	{
	public:
		/* Copy constructor. */
		Field(_In_ const Field&) = default;
		/* Move constructor. */
		Field(_In_ Field&&) = default;
		/* Releases the resources allocated by the field. */
		virtual ~Field(void) {}

		/* Copy assignment. */
		_Check_return_ Field& operator =(_In_ const Field&) = default;
		/* Move assignent. */
		_Check_return_ Field& operator =(_In_ Field&&) = default;

		/* Gets the SPIR-V information of this field. */
		_Check_return_ const FieldInfo& GetInfo(void) const
		{
			return *info;
		}

	protected:
		/* Initializes a new instance of a shader field. */
		Field(_In_ const FieldInfo &data)
			: info(&data)
		{}

		/* Checks if the specified type is accepted as input to the fields type. */
		_Check_return_ inline bool CheckInput(_In_ FieldType type) const
		{
			if (info->Type != type)
			{
				Log::Error("Cannot set field %s %s with %s value!", info->Type.GetName().c_str(), info->Name.c_str(), type.GetName().c_str());
				return false;
			}

			return true;
		}

	private:
		const FieldInfo *info;
	};
}