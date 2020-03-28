#include "Graphics/Lighting/DirectionalLight.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::DirectionalLight::DirectionalLight(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassDirectionalLight, layout), radiance(1.0f), envi(&GetDescriptor(DeferredRenderer::SubpassDirectionalLight, "Environment"))
{
	const Vector3 dir = normalize(Vector3(-0.7f));
	SetDirection(dir.X, dir.Y, dir.Z);
}

void Pu::DirectionalLight::Stage(byte * dest)
{
	/* The direction is given from the source to the target, but we need to store it the other way around. */
	const Vector3 dir = -GetDirection();

	Copy(dest, &dir);
	Copy(dest + sizeof(Vector4), &radiance);
}