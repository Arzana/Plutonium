#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Graphics/Vulkan/SPIR-V/SPIRV.h"
#include "Graphics/Vulkan/SPIR-V/FieldType.h"

namespace Pu
{
	/* Defines a pipeline specialization constant in a specific shader. */
	class SpecializationConstant
	{
	public:
		/* Sets the offset of this specialization constant in the constant buffer. */
		inline void SetOffset(_In_ uint32 value)
		{
			entry.Offset = value;
		}

		/* Gets the SPIR-V ID of this specialization constant. */
		_Check_return_ inline spv::Id GetID(void) const
		{
			return id;
		}

		/* Gets the ID of this specialization constant. */
		_Check_return_ inline uint32 GetConstantID(void) const
		{
			return entry.ConstantID;
		}

		/* Gets the name of this specialization constant. */
		_Check_return_ inline const string& GetName(void) const
		{
			return name;
		}

		/* Gets the type of this specialization constant. */
		_Check_return_ inline FieldType GetType(void) const
		{
			return type;
		}

		/* Gets the default value of the specialization constant. */
		template <typename value_t>
		_Check_return_ value_t GetDefaultValue(void) const
		{
			return static_cast<value_t>(defaultValue);
		}

	private:
		friend class Shader;
		friend class Pipeline;

		spv::Id id;
		string name;
		FieldType type;
		SpecializationMapEntry entry;
		double defaultValue;

		SpecializationConstant(spv::Id id, const string &name, FieldType type);
	};
}