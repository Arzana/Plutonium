#pragma once

namespace Pu
{
	/* Specifies the basic types that a Vulkan field can be. */
	enum class FieldTypes
	{
		/* The type is either unknown, invalid or not supported. */
		Invalid,
		/* A unsigned int with width 8. */
		Byte,
		/* A signed int with width 8. */
		SByte,
		/* A signed int with width 16. */
		Short,
		/* A unsigned int with width 16. */
		UShort,
		/* A signed int with width 32. */
		Int,
		/* A unsigned int with width 32. */
		UInt,
		/* A signed int with width 64. */
		Long,
		/* A unsigned int with width 64. */
		ULong,
		/* A 16 bit floating point. */
		HalfFloat,
		/* A 32 bit floating point. */
		Float,
		/* A 64 bit floating point. */
		Double,
		/* A 2D floating point vector. */
		Vec2,
		/* A 3D floating point vector. */
		Vec3,
		/* A 4D floating point vector. */
		Vec4,
		/* A 4x4 floating point matrix. */
		Matrix
	};

	/* Gets a human readable version of a field type. */
	_Check_return_ inline const char* to_string(_In_ FieldTypes type)
	{
		switch (type)
		{
		case Pu::FieldTypes::Byte:
			return "uint8";
		case Pu::FieldTypes::SByte:
			return "int8";
		case Pu::FieldTypes::Short:
			return "Int16";
		case Pu::FieldTypes::UShort:
			return "Uint16";
		case Pu::FieldTypes::Int:
			return "Int32";
		case Pu::FieldTypes::UInt:
			return "Uint32";
		case Pu::FieldTypes::Long:
			return "Int64";
		case Pu::FieldTypes::ULong:
			return "UInt64";
		case Pu::FieldTypes::HalfFloat:
			return "Float16";
		case Pu::FieldTypes::Float:
			return "Float32";
		case Pu::FieldTypes::Double:
			return "Float64";
		case Pu::FieldTypes::Vec2:
			return "Vector2";
		case Pu::FieldTypes::Vec3:
			return "Vector3";
		case Pu::FieldTypes::Vec4:
			return "Vector4";
		case Pu::FieldTypes::Matrix:
			return "Matrix";
		default:
			return "Unknown";
		}
	}
}