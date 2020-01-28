#include "Graphics/Vulkan/Shaders/PushConstant.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

void Pu::PushConstant::SetOffset(size_t offset)
{
	if (offset < range.Size) range.Offset = static_cast<uint32>(offset);
	else Log::Error("Attempting to set offset of '%s' to an invalid offset!", GetInfo().Name.c_str());
}

Pu::PushConstant::PushConstant(const FieldInfo & data)
	: Field(data), maxRange(0)
{}

Pu::PushConstant::PushConstant(const PhysicalDevice & physicalDevice, const FieldInfo & data, ShaderStageFlag stage)
	: Field(data), maxRange(physicalDevice.GetLimits().MaxPushConstantsSize)
{
	range.StageFlags = stage;
	range.Offset = data.Decorations.Numbers.at(spv::Decoration::Offset);
	range.Size = static_cast<uint32>(Field::GetSize());

	if (range.Size > maxRange)
	{
		Log::Error("Push constant '%s' is larger than the maximum push constant size!", data.Name.c_str());
	}
}