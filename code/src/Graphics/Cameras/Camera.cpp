#include "Graphics/Cameras/Camera.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Physics/Systems/ShapeTests.h"
#include "Application.h"
#include <imgui/include/imgui.h>

Pu::Camera::Camera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass)
	: DescriptorSetGroup(pool), viewDirty(false), window(&wnd),
	exposure(1.0f), brightness(1.0f), contrast(1.0f), saturation(1.0f)
{
	wnd.OnSizeChanged.Add(*this, &Camera::OnWindowResize);
	OnWindowResize(wnd, ValueChangedEventArgs<Vector2>(Vector2(), Vector2()));

	/* All of the camera descriptor sets use set ID 0. */
	offsetSp0 = Add(DeferredRenderer::SubpassTerrain, renderpass.GetSubpass(DeferredRenderer::SubpassTerrain).GetSetLayout(0));
	offsetSp1 = Add(DeferredRenderer::SubpassAdvancedStaticGeometry, renderpass.GetSubpass(DeferredRenderer::SubpassAdvancedStaticGeometry).GetSetLayout(0));
	offsetSp2 = Add(DeferredRenderer::SubpassDirectionalLight , renderpass.GetSubpass(DeferredRenderer::SubpassDirectionalLight).GetSetLayout(0));
	offsetSp3 = Add(DeferredRenderer::SubpassSkybox, renderpass.GetSubpass(DeferredRenderer::SubpassSkybox).GetSetLayout(0));
	offsetSp4 = Add(DeferredRenderer::SubpassPostProcessing, renderpass.GetSubpass(DeferredRenderer::SubpassPostProcessing).GetSetLayout(0));
	offsetSp5 = Add(DeferredRenderer::SubpassPointLight, renderpass.GetSubpass(DeferredRenderer::SubpassPointLight).GetSetLayout(0));
	offsetSp6 = offsetSp5 + GetOffsetAligned(sizeof(Matrix) << 1);
}

Pu::Camera::Camera(Camera && value)
	: DescriptorSetGroup(std::move(value)), pos(value.pos), window(value.window),
	view(value.view), proj(value.proj), iproj(value.iproj), iview(value.iview),
	exposure(value.exposure), brightness(value.brightness), contrast(value.contrast),
	wndSize(value.wndSize), viewDirty(value.viewDirty), Orientation(value.Orientation),
	offsetSp0(value.offsetSp0), offsetSp1(value.offsetSp1), offsetSp2(value.offsetSp2),
	offsetSp3(value.offsetSp3), offsetSp4(value.offsetSp4), offsetSp5(value.offsetSp5),
	offsetSp6(value.offsetSp6)
{
	window->OnSizeChanged.Add(*this, &Camera::OnWindowResize);
}

Pu::Camera & Pu::Camera::operator=(Camera && other)
{
	if (this != &other)
	{
		Destroy();
		DescriptorSetGroup::operator=(std::move(other));

		pos = other.pos;
		view = other.view;
		proj = other.proj;
		iproj = other.iproj;
		iview = other.iview;
		Orientation = other.Orientation;
		exposure = other.exposure;
		brightness = other.brightness;
		contrast = other.contrast;
		wndSize = other.wndSize;
		window = other.window;
		viewDirty = other.viewDirty;
		offsetSp0 = other.offsetSp0;
		offsetSp1 = other.offsetSp1;
		offsetSp2 = other.offsetSp2;
		offsetSp3 = other.offsetSp3;
		offsetSp4 = other.offsetSp4;
		offsetSp5 = other.offsetSp5;
		offsetSp6 = other.offsetSp6;

		window->OnSizeChanged.Add(*this, &Camera::OnWindowResize);
	}

	return *this;
}

Pu::Vector3 Pu::Camera::ScreenToWorldRay(Vector2 v) const
{
	/* Just convert to NDC and then use the default function. */
	return NDCToWorldRay(ToNDC(v));
}

Pu::Vector3 Pu::Camera::ScreenToWorld(Vector2 v, float z) const
{
	/* Just convert from screen space to normalized device coordinate and use the default function. */
	const Vector2 ndc = ToNDC(v);
	return NDCToWorld(Vector3(ndc.X, ndc.Y, z));
}

Pu::Vector3 Pu::Camera::NDCToWorldRay(Vector2 v) const
{
	/* Construct the full normalized device coordinate. */
	const Vector4 ndc = Vector4(v, 0.0f, 0.0f);

	/* Convert to eye space and perform perspective division. */
	Vector4 eye = iproj * ndc;
	eye /= eye.W;

	/* Convert to world space and return the normalized direction from the position on the near plane to the camera. */
	const Vector4 world = GetInverseView() * eye;
	return normalize(world.XYZ - pos);
}

Pu::Vector3 Pu::Camera::NDCToWorld(Vector3 v) const
{
	/* Construct the full normalized device coordinate. */
	const Vector4 ndc = Vector4(v, 1.0f);

	/* Convert to eye space and perform perspective division. */
	Vector4 eye = iproj * ndc;
	eye /= eye.W;

	/* The w component can be discarded because no world space coordinate would have one. */
	return (GetInverseView() * eye).XYZ;
}

const Pu::Matrix & Pu::Camera::GetInverseView(void) const
{
	if (viewDirty)
	{
		iview = view.GetInverse();
		viewDirty = false;
	}

	return iview;
}

bool Pu::Camera::Cull(const AABB & boundingBox) const
{
	return !intersects(frustum, boundingBox);
}

bool Pu::Camera::Cull(const AABB & boundingBox, const Matrix & transform) const
{
	return !intersects(frustum, boundingBox * transform);
}

void Pu::Camera::Visualize(void)
{
	if constexpr (ImGuiAvailable)
	{
		ImGui::PushID(this);
		if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			VisualizeInternal();
			ImGui::End();
		}
		ImGui::PopID();
	}
}

void Pu::Camera::SetView(const Matrix & value)
{
	view = value;
	viewDirty = true;
	frustum = Frustum{ GetViewProjection() };
}

void Pu::Camera::SetProjection(const Matrix & value)
{
	proj = value;
	iproj = proj.GetInverse();
	frustum = Frustum{ GetViewProjection() };
}

void Pu::Camera::Stage(DescriptorPool&, byte * dest)
{
	/* Stage the view, projection, viewport and frustum for the terrain subpass. */
	Copy(dest + offsetSp0, &proj);
	Copy(dest + offsetSp0 + sizeof(Matrix), &view);
	Copy(dest + offsetSp0 + (sizeof(Matrix) << 1), &frustum);
	Copy(dest + offsetSp0 + (sizeof(Matrix) << 1) + sizeof(Frustum), &wndSize);

	/* Stage the view and projection matrix for the static subpass. */
	Copy(dest + offsetSp1, &proj);
	Copy(dest + offsetSp1 + sizeof(Matrix), &view);

	/* Stage the inverse view, projection, and the camera position to the directional light. */
	Copy(dest + offsetSp2, &iproj);
	Copy(dest + offsetSp2 + sizeof(Matrix), &GetInverseView());
	Copy(dest + offsetSp2 + (sizeof(Matrix) << 1), &pos);

	/* Stage the inverse projection matrix for the skybox subpass. */
	Copy(dest + offsetSp3, &iproj);
	Copy(dest + offsetSp3 + sizeof(Matrix), &iview);
	Copy(dest + offsetSp3 + (sizeof(Matrix) << 1), &pos);

	/* Stage the exposure, brightness, contrast and saturation to the camera and final subpass. */
	Copy(dest + offsetSp4, &exposure);
	Copy(dest + offsetSp4 + sizeof(float), &brightness);
	Copy(dest + offsetSp4 + sizeof(Vector2), &contrast);
	Copy(dest + offsetSp4 + sizeof(Vector3), &saturation);

	/* Stage the projection, view, inverse projection, inverse view and position to the point light. */
	Copy(dest + offsetSp5, &proj);
	Copy(dest + offsetSp5 + sizeof(Matrix), &view);
	Copy(dest + offsetSp6, &iproj);
	Copy(dest + offsetSp6 + sizeof(Matrix), &iview);
	Copy(dest + offsetSp6 + (sizeof(Matrix) << 1), &pos);
}

void Pu::Camera::Destroy(void)
{
	window->OnSizeChanged.Remove(*this, &Camera::OnWindowResize);
}

Pu::Vector2 Pu::Camera::ToNDC(Vector2 v) const
{
	return v / wndSize * 2.0f - Vector2(1.0f);
}

void Pu::Camera::OnWindowResize(const NativeWindow&, ValueChangedEventArgs<Vector2>)
{
	const Viewport vp = window->GetClientBounds();
	wndSize.X = vp.Width;
	wndSize.Y = vp.Height;
}

void Pu::Camera::VisualizeInternal(void)
{
	ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f);
	ImGui::SliderFloat("Brightness", &brightness, 0.0f, 2.0f);
	ImGui::SliderFloat("Contrast", &contrast, 0.0f, 3.0f);
	ImGui::SliderFloat("Saturation", &saturation, 0.0f, 2.0f);
}