#include "Graphics/Lighting/PointLightPool.h"

Pu::PointLightPool::PointLightPool(LogicalDevice & device, uint32 maxLights)
	: DynamicBuffer(device, sizeof(PointLight) * maxLights, BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst), isDirty(false)
{
	buffer.reserve(maxLights);
}

float Pu::PointLightPool::GetLightRadius(Vector3 color, float intensity, float falloffLinaer, float falloffQuadratic, uint8 cutoff)
{
#ifdef _DEBUG
	if (intensity < 0.0f) Log::Fatal("Point light constant falloff must be greater than zero!");
	if (falloffLinaer <= 0.0f) Log::Fatal("Point light linear falloff should be greater than zero (light gains energy)!");
	if (falloffQuadratic <= 0.0f) Log::Fatal("Point light quadratic falloff should be greater than zero (light gains energy)!");
#endif

	const float lMin = maxv<byte>() / (cutoff / intensity);
	const float lMax = max(color.X, color.Y, color.Z);
	return (-falloffLinaer + sqrtf(sqr(falloffLinaer) - 4.0f * falloffQuadratic * (1.0f - lMax * lMin))) / (2.0f * falloffQuadratic);
}

void Pu::PointLightPool::AddLight(Vector3 position, Color color, float intensity, float falloffLinaer, float falloffQuadratic)
{
#ifdef _DEBUG
	if (!HasSpace()) Log::Fatal("Unable to add point light to pool (out of memory)!");
#endif

	/* Calculate the light's radius. */
	const Vector3 clr = color.ToVector3();
	const float r = GetLightRadius(clr, intensity, falloffLinaer, falloffQuadratic);

	/* Add the point light information to our buffer. */
	buffer.emplace_back(PointLight
		{
			Matrix::CreateScaledTranslation(position, r),
			clr,
			intensity,
			falloffLinaer,
			falloffQuadratic
		});

	isDirty = true;
}

void Pu::PointLightPool::Update(CommandBuffer & cmdBuffer)
{
	if (isDirty) StageLightBuffer();
	DynamicBuffer::Update(cmdBuffer);
}

void Pu::PointLightPool::StageLightBuffer(void)
{
	isDirty = false;

	BeginMemoryTransfer();
	PointLight *memory = reinterpret_cast<PointLight*>(GetHostMemory());

	for (const PointLight &cur : buffer)
	{
		memcpy(memory, &cur, sizeof(PointLight));
		++memory;
	}

	EndMemoryTransfer();
}