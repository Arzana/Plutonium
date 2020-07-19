#include "Graphics/Lighting/PointLightPool.h"

Pu::PointLightPool::PointLightPool(LogicalDevice & device, uint32 maxLights)
	: DynamicBuffer(device, sizeof(PointLight) * maxLights, BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst)
{
	buffer.reserve(maxLights);
}

void Pu::PointLightPool::AddLight(Vector3 position, Color color, float attenuationConstant, float attenuationLinear, float attenuationQuadratic)
{
#ifdef _DEBUG
	if (!HasSpace()) Log::Fatal("Unable to add point light to pool (out of memory)!");
#endif

	const Vector3 clr = color.ToVector3();

	/* Calculate the light's radius. */
	constexpr float cutoff = 255.0f / 5.0f;
	const float l = max(max(clr.X, clr.Y), clr.Z);
	const float r = (-attenuationLinear + sqrtf(sqr(attenuationLinear) - 4.0f * attenuationQuadratic * (attenuationConstant - l * cutoff))) / (2.0f * attenuationQuadratic);

	/* Add the point light information to our buffer. */
	buffer.emplace_back(PointLight
		{
			Matrix::CreateScaledTranslation(position, r),
			clr,
			attenuationConstant,
			attenuationLinear,
			attenuationQuadratic
		});

	StageLightBuffer();
}

void Pu::PointLightPool::StageLightBuffer(void)
{
	BeginMemoryTransfer();
	PointLight *memory = reinterpret_cast<PointLight*>(GetHostMemory());

	for (const PointLight &cur : buffer)
	{
		memcpy(memory, &cur, sizeof(PointLight));
		++memory;
	}

	EndMemoryTransfer();
}