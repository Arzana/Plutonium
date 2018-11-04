#pragma once
#include <sal.h>
#include "FieldTypes.h"
#include "Core\Math\Constants.h"

namespace Plutonium
{
	/* Defines a base for a shader's field. */
	class Field
	{
		Field(_In_ const Field &value) = delete;
		Field(_In_ Field &&value) = delete;

		_Check_return_ Field& operator =(_In_ const Field &other) = delete;
		_Check_return_ Field& operator = (_In_ Field &&other) = delete;

	protected:
		friend class Shader;

		/* The ID for this field. */
		uint32 ptr;
		/* The name of this field. */
		const char *name;
		/* The type of this field. */
		FieldType type;

		/* Initializes a new instance of a non operational field. */
		Field(void);
		/* Initializes a new instance of a OpenGL shader field. */
		Field(_In_ uint32 ptr, _In_ const char *name, _In_ int32 type);

		/* Checks whether this field is of a required type and throws if this is not the case. */
		void CheckInput(_In_ FieldType requiredType);
		/* Returns whether this field is a attribute or a uniform. */
		_Check_return_ virtual bool IsAttribute(void) const = 0;
	};
}