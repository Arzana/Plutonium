#include "Graphics\Rendering\Field.h"
#include "Core\Diagnostics\Logging.h"

Plutonium::Field::Field(void)
	: ptr(0), name("INVALID"), type(FieldType::Invalid)
{}

Plutonium::Field::Field(uint32 ptr, const char * name, int32 type)
	: ptr(ptr), name(name), type(static_cast<FieldType>(type))
{}

void Plutonium::Field::CheckInput(FieldType requiredType)
{
	/* Check if field is valid and if the types match. */
	LOG_THROW_IF(ptr == -1, "Cannot set to invalid shader value!");
	LOG_THROW_IF(requiredType != type, "Cannot set %s %s(%d) to %s value!", _CrtGetFieldVisualType(type), name, ptr, _CrtGetFieldVisualType(requiredType));
}