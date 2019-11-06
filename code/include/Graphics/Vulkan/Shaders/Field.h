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
		_Check_return_ inline const FieldInfo& GetInfo(void) const
		{
			return *info;
		}

		/* 
		Gets the size of the field as if it was in a uniform buffer. 
		https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
		The size of a value in a uniform buffer is determined as follows:
		Scalars (int, float, bool, etc)			4 bytes										Called N for further computations
		Vector									2 or 4 * N bytes							8 bytes for vec2 and 16 bytes for vec3 and vec4
		Arrays									Element Count * 16 bytes					Stored as multiple vec4's
		Matrices								Column Count * 16 bytes						Stored as array of column vec4's
		Struct									Sum(Components), Alligned to 16 bytes		Stored as the sum of it's components but padded to be 16 byte alligned.
		*/
		_Check_return_ inline DeviceSize GetSize(void) const
		{
			if (info->ArrayElements == 1) return info->Type.GetSize();
			else return max(16ull, info->Type.GetSize() * info->ArrayElements);
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