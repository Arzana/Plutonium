#include "Graphics/Vulkan/Shaders/Output.h"

void Pu::Output::SetDescription(const Swapchain & swapchain)
{
	description.Format = swapchain.GetImageFormat();
	description.InitialLayout = ImageLayout::PresentSrcKhr;
	description.FinalLayout = ImageLayout::PresentSrcKhr;
}

Pu::Output::Output(const FieldInfo & data, uint32 attachment)
	: Field(data), type(OutputUsage::Color), resolve(false),
	reference(attachment, ImageLayout::General), description(Format::Undefined, ImageLayout::General, ImageLayout::General)
{
	if (data.Storage != spv::StorageClass::Output) Log::Fatal("The output class cannot be used to store '%s'!", to_string(data.Storage));
}