#include "Graphics/Vulkan/Shaders/SpecializationConstant.h"

Pu::SpecializationConstant::SpecializationConstant(spv::Id id, const string & name, FieldType type)
	: id(id), name(name), type(type), entry(0, static_cast<uint32>(type.GetSize()))
{}