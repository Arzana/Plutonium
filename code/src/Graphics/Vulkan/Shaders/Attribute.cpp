#include "Graphics/Vulkan/Shaders/Attribute.h"

Pu::Attribute::Attribute(const FieldInfo & data)
	: Field(data)
{
	/* Pre-set location. */
	description.Location = data.Location;

	/* Pre-set format. */
	switch (data.Type)
	{
	case (FieldTypes::Byte):
		description.Format = Format::R8_UINT;
		break;
	case (FieldTypes::SByte):
		description.Format = Format::R8_SINT;
		break;
	case (FieldTypes::Short):
		description.Format = Format::R16_SINT;
		break;
	case (FieldTypes::UShort):
		description.Format = Format::R16_UINT;
		break;
	case (FieldTypes::Int):
		description.Format = Format::R32_SINT;
		break;
	case (FieldTypes::UInt):
		description.Format = Format::R32_UINT;
		break;
	case (FieldTypes::Long):
		description.Format = Format::R64_SINT;
		break;
	case (FieldTypes::ULong):
		description.Format = Format::R64_UINT;
		break;
	case (FieldTypes::HalfFloat):
		description.Format = Format::R16_SFLOAT;
		break;
	case (FieldTypes::Float):
		description.Format = Format::R32_SFLOAT;
		break;
	case (FieldTypes::Double):
		description.Format = Format::R64_SFLOAT;
		break;
	case (FieldTypes::Vec2):
		description.Format = Format::R32G32_SFLOAT;
		break;
	case (FieldTypes::Vec3):
		description.Format = Format::R32G32B32_SFLOAT;
		break;
	case (FieldTypes::Vec4):
		description.Format = Format::R32G32B32A32_SFLOAT;
		break;
	default:
		Log::Fatal("Unknown attribute type!");
		break;
	}
}