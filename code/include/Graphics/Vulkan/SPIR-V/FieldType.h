#pragma once
#include "Core/String.h"
#include "Graphics/Vulkan/VulkanEnums.h"

namespace Pu
{
	/* Defines the component type of a field type. */
	enum class ComponentType
	{
		/* The type is either unknown, invalid or not supported. */
		Invalid,
		/* A boolean value. */
		Bool,
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
		/* A sampled image. */
		SampledImage,
		/* A storage image. */
		StoreImage
	};

	/* Specifies the dimension of a field type. */
	enum class SizeType
	{
		/* A single scalar value or 1D image. */
		Scalar,
		/* A two dimensional value or 2D image. */
		Vector2,
		/* A three dimensional value or a 3D image. */
		Vector3,
		/* A four dimensional value. */
		Vector4,
		/* A 2x2 matrix. */
		Matrix2,
		/* A 3x3 matrix. */
		Matrix3,
		/* A 4x4 matrix. */
		Matrix4,
		/* A cube map image. */
		Cube
	};

	/* Specifies the basic types that a Vulkan field can be. */
	struct FieldType
	{
	public:
		/* Specifies the type of the field. */
		ComponentType ComponentType;
		/* Specified the size of the field. */
		SizeType ContainerType;
		/* Specified the amount of element in the field type. */
		uint32 Length;

		/* Initializes an empty instance of a field type. */
		FieldType(void);
		/* Initializes a new instance of a field type. */
		FieldType(_In_ Pu::ComponentType component, _In_ Pu::SizeType size);
		/* Copy constructor. */
		FieldType(_In_ const FieldType&) = default;
		/* Move constructor. */
		FieldType(_In_ FieldType&&) = default;

		/* Copy assignment. */
		_Check_return_ FieldType& operator =(_In_ const FieldType&) = default;
		/* Move assignment. */
		_Check_return_ FieldType& operator =(_In_ FieldType&&) = default;
		/* Checks whether two field types are equal. */
		_Check_return_ bool operator ==(_In_ const FieldType &other) const;
		/* Checks whether two field types differ. */
		_Check_return_ bool operator !=(_In_ const FieldType &other) const;

		/* Gets the size (in bytes) of this field type. */
		_Check_return_ size_t GetSize(void) const;
		/* Gets a displayable name of this field type. */
		_Check_return_ string GetName(void) const;
		/* Gets the Vulkan format that represents this field type. */
		_Check_return_ Format GetFormat(void) const;
		/* Gets whether the specified field is an image. */
		_Check_return_ bool IsImage(void) const;
		/* Gets whether the specified field is a matrix. */
		_Check_return_ bool IsMatrix(void) const;
	};
}