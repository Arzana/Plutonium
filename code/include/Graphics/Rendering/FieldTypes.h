#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Specifies the basic types that a OpenGL field can be. */
	enum class FieldType
	{
		/* The types is either unknown or invalid. */
		Invalid = -1,
		/* A boolean value. */
		Bool = GL_BOOL,
		/* A unsigned int8. */
		Byte = GL_UNSIGNED_BYTE,
		/* A signed int8. */
		SByte = GL_BYTE,
		/* A signed int16. */
		Short = GL_SHORT,
		/* A unsigned int16. */
		UShort = GL_UNSIGNED_SHORT,
		/* A signed int32. */
		Int = GL_INT,
		/* A unsigned int32. */
		UInt = GL_UNSIGNED_INT,
		/* A 16 bit floating point. */
		HalfFloat = GL_HALF_FLOAT,
		/* A 32 bit floating point. */
		Float = GL_FLOAT,
		/* A 64 bit floating point. */
		Double = GL_DOUBLE,
		/* A 2D floating point vector. */
		Vect2 = GL_FLOAT_VEC2,
		/* A 3D floating point vector. */
		Vect3 = GL_FLOAT_VEC3,
		/* A 4D floating point vector. */
		Vect4 = GL_FLOAT_VEC4,
		/* A 4x4 floating point matrix. */
		Matrix = GL_FLOAT_MAT4,
		/* A 2D texture. */
		Texture = GL_SAMPLER_2D,
		/* A cube map texture. */
		CubeTexture = GL_SAMPLER_CUBE
	};

	/* Gets a string version of a specified OpenGL field type. */
	_Check_return_ inline const char* _CrtGetFieldVisualType(_In_ FieldType type)
	{
		switch (type)
		{
			case Plutonium::FieldType::Bool:
				return "Bool";
			case Plutonium::FieldType::Byte:
				return "Byte";
			case Plutonium::FieldType::SByte:
				return "SByte";
			case Plutonium::FieldType::Short:
				return "Short";
			case Plutonium::FieldType::UShort:
				return "UShort";
			case Plutonium::FieldType::Int:
				return "Int";
			case Plutonium::FieldType::UInt:
				return "UInt";
			case Plutonium::FieldType::HalfFloat:
				return "Float16";
			case Plutonium::FieldType::Float:
				return "Float";
			case Plutonium::FieldType::Double:
				return "Float64";
			case Plutonium::FieldType::Vect2:
				return "Vector2";
			case Plutonium::FieldType::Vect3:
				return "Vector3";
			case Plutonium::FieldType::Vect4:
				return "Vector4";
			case Plutonium::FieldType::Matrix:
				return "Matrix";
			case Plutonium::FieldType::Texture:
				return "Texture";
			default:
				return "UNKNOWN";
		}
	}
}