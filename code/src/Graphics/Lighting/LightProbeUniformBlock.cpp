#include "Graphics/Lighting/LightProbeUniformBlock.h"

Pu::LightProbeUniformBlock::LightProbeUniformBlock(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, 0, layout)
{}

void Pu::LightProbeUniformBlock::Stage(byte * dest)
{
	for (size_t i = 0; i < 6; i++)
	{
		Copy(dest + sizeof(Matrix) * i, views + i);
	}
}