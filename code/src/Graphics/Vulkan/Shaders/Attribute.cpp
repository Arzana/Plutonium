#include "Graphics/Vulkan/Shaders/Attribute.h"

Pu::Attribute::Attribute(const FieldInfo & data)
	: Field(data)
{
	/* Pre-set location and format. */
	description.Location = data.GetLocation();
	description.Format = data.Type.GetFormat();
}