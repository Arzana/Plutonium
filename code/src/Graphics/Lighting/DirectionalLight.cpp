#include "Graphics/Lighting/DirectionalLight.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include <imgui/include/imgui.h>

Pu::DirectionalLight::DirectionalLight(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassDirectionalLight, layout), radiance(1.0f), envi(&GetDescriptor(DeferredRenderer::SubpassDirectionalLight, "Environment"))
{}

void Pu::DirectionalLight::RenderGUI(void)
{
	if constexpr (ImGuiAvailable)
	{
		if (ImGui::Begin("Directional Light"))
		{
			ImGui::SliderFloat("Intensity", &radiance.W, 0.0f, 10.0f);
			ImGui::ColorPicker3("Color", radiance.f);
			ImGui::End();
		}
	}
}

void Pu::DirectionalLight::Stage(byte * dest)
{
	/* The direction is given from the source to the target, but we need to store it the other way around. */
	const Vector3 dir = -GetDirection();

	Copy(dest, &dir);
	Copy(dest + sizeof(Vector4), &radiance);
}