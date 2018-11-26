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
}