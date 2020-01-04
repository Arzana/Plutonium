#include "Graphics/Lighting/LightProbeUniformBlock.h"

Pu::LightProbeUniformBlock::LightProbeUniformBlock(DescriptorPool & pool)
	: UniformBlock(pool, false)
{}

void Pu::LightProbeUniformBlock::Stage(byte * dest)
{
	for (size_t i = 0; i < 6; i++)
	{
		Copy(dest + sizeof(Matrix) * i, views + i);
	}
}