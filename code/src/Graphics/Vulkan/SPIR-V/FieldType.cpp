#include "Graphics/Vulkan/SPIR-V/FieldType.h"

Pu::FieldType::FieldType(void)
	: ComponentType(ComponentType::Invalid), ContainerType(SizeType::Scalar), Length(1)
{}

Pu::FieldType::FieldType(Pu::ComponentType component, Pu::SizeType size)
	: ComponentType(component), ContainerType(size), Length(1)
{
#ifdef _DEBUG
	if (ComponentType != ComponentType::SampledImage &&
		ComponentType != ComponentType::StoreImage
		&& ContainerType == SizeType::Cube)
	{
		Log::Fatal("Only images can use the Cube size type!");
	}

	if (IsImage() &&
		ContainerType != SizeType::Scalar &&
		ContainerType != SizeType::Vector2 &&
		ContainerType != SizeType::Vector3 &&
		ContainerType != SizeType::Cube)
	{
		Log::Fatal("Images can only be represented using Scalar, Vector2, Vector3 or Cube!");
	}
#endif
}

bool Pu::FieldType::operator==(const FieldType & other) const
{
	return other.ComponentType == ComponentType && other.ContainerType == ContainerType;
}

bool Pu::FieldType::operator!=(const FieldType & other) const
{
	return other.ComponentType != ComponentType || other.ContainerType != ContainerType;
}

size_t Pu::FieldType::GetSize(void) const
{
	size_t result = 0;

	switch (ComponentType)
	{
	case ComponentType::Byte:
	case ComponentType::SByte:
	case ComponentType::Short:
	case ComponentType::UShort:
	case ComponentType::HalfFloat:
	case ComponentType::Int:
	case ComponentType::UInt:
	case ComponentType::Float:
		result = 4;
		break;
	case ComponentType::Long:
	case ComponentType::ULong:
	case ComponentType::Double:
		result = 8;
		break;
	case ComponentType::SampledImage:
	case ComponentType::StoreImage:
		Log::Fatal("Cannot request size of image field!");
		break;
	case ComponentType::Invalid:
	default:
		Log::Fatal("Cannot get size of invalid or unknown field type!");
	}

	/* std140 can't allign 3D vectors or matrices so they're alligned the same as 4D. */
	switch (ContainerType)
	{
	case SizeType::Scalar:
		break;
	case SizeType::Vector2:
		result <<= 1;
		break;
	case SizeType::Vector3:
	case SizeType::Vector4:
	case SizeType::Matrix2:
		result <<= 2;
		break;
	case SizeType::Matrix3:
	case SizeType::Matrix4:
		result <<= 4;
		break;
	case SizeType::Cube:
	default:
		Log::Fatal("Cannot request size of cube image field!");
		break;
	}

	return result * Length;
}

Pu::string Pu::FieldType::GetName(void) const
{
	/* Images use a special format so do them first. */
	if (IsImage())
	{
		switch (ContainerType)
		{
		case SizeType::Scalar:
			return "1D Image";
		case SizeType::Vector2:
			return "2D Image";
		case SizeType::Vector3:
			return "3D Image";
		case SizeType::Cube:
			return "Cubemap Image";
		}
	}

	string result;

	switch (ComponentType)
	{
	case ComponentType::Invalid:
		return "Invalid";
	case ComponentType::Byte:
		result = "UInt8";
		break;
	case ComponentType::SByte:
		result = "Int8";
		break;
	case ComponentType::Short:
		result = "Int16";
		break;
	case ComponentType::UShort:
		result = "UInt16";
		break;
	case ComponentType::Int:
		result = "Int";
		break;
	case ComponentType::UInt:
		result = "UInt";
		break;
	case ComponentType::Long:
		result = "Int64";
		break;
	case ComponentType::ULong:
		result = "UInt64";
		break;
	case ComponentType::HalfFloat:
		result = "Float16";
		break;
	case ComponentType::Float:
		result = "Float";
		break;
	case ComponentType::Double:
		result = "Double";
		break;
	default:
		return "Unknown";
	}

	switch (ContainerType)
	{
	case SizeType::Scalar:
		break;
	case SizeType::Vector2:
		if (ComponentType == ComponentType::Float) result = "Vector2";
		else result += " Vector2";
		break;
	case SizeType::Vector3:
		if (ComponentType == ComponentType::Float) result = "Vector3";
		else result += " Vector3";
		break;
	case SizeType::Vector4:
		if (ComponentType == ComponentType::Float) result = "Vector4";
		else result += " Vector4";
		break;
	case SizeType::Matrix2:
		if (ComponentType == ComponentType::Float) result = "2x2 Matrix";
		else result += " 2x2 Matrix";
		break;
	case SizeType::Matrix3:
		if (ComponentType == ComponentType::Float) result = "3x3 Matrix";
		else result += " 3x3 Matrix";
		break;
	case SizeType::Matrix4:
		if (ComponentType == ComponentType::Float) result = "Matrix";
		else result += " Matrix";
		break;
	default:
		return "Unknown";
	}

	if (Length > 1) return result + " Array";
	else return result;
}

Pu::Format Pu::FieldType::GetFormat(void) const
{
	switch (ContainerType)
	{
	case SizeType::Scalar:
		switch (ComponentType)
		{
		case ComponentType::Byte:
			return Format::R8_UINT;
		case ComponentType::SByte:
			return Format::R8_SINT;
		case ComponentType::Short:
			return Format::R16_SINT;
		case ComponentType::UShort:
			return Format::R16_UINT;
		case ComponentType::Int:
			return Format::R32_SINT;
		case ComponentType::UInt:
			return Format::R32_UINT;
		case ComponentType::Long:
			return Format::R64_SINT;
		case ComponentType::ULong:
			return Format::R64_UINT;
		case ComponentType::HalfFloat:
			return Format::R16_SFLOAT;
		case ComponentType::Float:
			return Format::R32_SFLOAT;
		case ComponentType::Double:
			return Format::R64_SFLOAT;
		default:
			return Format::Undefined;
		}
		break;
	case SizeType::Vector2:
		switch (ComponentType)
		{
		case ComponentType::Byte:
			return Format::R8G8_UINT;
		case ComponentType::SByte:
			return Format::R8G8_SINT;
		case ComponentType::Short:
			return Format::R16G16_SINT;
		case ComponentType::UShort:
			return Format::R16G16_UINT;
		case ComponentType::Int:
			return Format::R32G32_SINT;
		case ComponentType::UInt:
			return Format::R32G32_UINT;
		case ComponentType::Long:
			return Format::R64G64_SINT;
		case ComponentType::ULong:
			return Format::R64G64_UINT;
		case ComponentType::HalfFloat:
			return Format::R16G16_SFLOAT;
		case ComponentType::Float:
			return Format::R32G32_SFLOAT;
		case ComponentType::Double:
			return Format::R64G64_SFLOAT;
		default:
			return Format::Undefined;
		}
		break;
	case SizeType::Vector3:
		switch (ComponentType)
		{
		case ComponentType::Byte:
			return Format::R8G8B8_UINT;
		case ComponentType::SByte:
			return Format::R8G8B8_SINT;
		case ComponentType::Short:
			return Format::R16G16B16_SINT;
		case ComponentType::UShort:
			return Format::R16G16B16_UINT;
		case ComponentType::Int:
			return Format::R32G32B32_SINT;
		case ComponentType::UInt:
			return Format::R32G32B32_UINT;
		case ComponentType::Long:
			return Format::R64G64B64_SINT;
		case ComponentType::ULong:
			return Format::R64G64B64_UINT;
		case ComponentType::HalfFloat:
			return Format::R16G16B16_SFLOAT;
		case ComponentType::Float:
			return Format::R32G32B32_SFLOAT;
		case ComponentType::Double:
			return Format::R64G64B64_SFLOAT;
		default:
			return Format::Undefined;
		}
		break;
	case SizeType::Vector4:
		switch (ComponentType)
		{
		case ComponentType::Byte:
			return Format::R8G8B8A8_UINT;
		case ComponentType::SByte:
			return Format::R8G8B8A8_SINT;
		case ComponentType::Short:
			return Format::R16G16B16A16_SINT;
		case ComponentType::UShort:
			return Format::R16G16B16A16_UINT;
		case ComponentType::Int:
			return Format::R32G32B32A32_SINT;
		case ComponentType::UInt:
			return Format::R32G32B32A32_UINT;
		case ComponentType::Long:
			return Format::R64G64B64A64_SINT;
		case ComponentType::ULong:
			return Format::R64G64B64A64_UINT;
		case ComponentType::HalfFloat:
			return Format::R16G16B16A16_SFLOAT;
		case ComponentType::Float:
			return Format::R32G32B32A32_SFLOAT;
		case ComponentType::Double:
			return Format::R64G64B64A64_SFLOAT;
		default:
			return Format::Undefined;
		}
		break;
	case SizeType::Matrix2:
		return Format::R32G32_SFLOAT;
	case SizeType::Matrix3:
		return Format::R32G32B32_SFLOAT;
	case SizeType::Matrix4:
		return Format::R32G32B32A32_SFLOAT;
	case SizeType::Cube:
	default:
		return Format::Undefined;
	}
}

bool Pu::FieldType::IsImage(void) const
{
	return ComponentType == ComponentType::SampledImage || ComponentType == ComponentType::StoreImage;
}

bool Pu::FieldType::IsMatrix(void) const
{
	return ContainerType == SizeType::Matrix2 || ContainerType == SizeType::Matrix3 || ContainerType == SizeType::Matrix4;
}
