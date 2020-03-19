#include "Graphics/Lighting/DirectionalLight.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::DirectionalLight::DirectionalLight(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassDirectionalLight, layout), radiance(1.0f), intensity(1.0f), envi(&GetDescriptor(DeferredRenderer::SubpassDirectionalLight, "Environment"))
{
	const Vector3 dir = normalize(Vector3(0.7f));
	SetDirection(dir.X, dir.Y, dir.Z);
}

void Pu::DirectionalLight::Stage(byte * dest)
{
	const Vector3 dir = GetDirection();
	const Vector3 color = radiance * intensity;

	Copy(dest, &dir);
	Copy(dest + sizeof(Vector4), &color);
}