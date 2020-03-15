#include "Graphics/Cameras/Camera.h"
#include "Application.h"

Pu::Camera::Camera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass)
	: DescriptorSetGroup(pool), viewDirty(false),
	exposure(1.0f), brightness(0.0f), contrast(1.0f),
	window(&wnd)
{
	wnd.OnSizeChanged.Add(*this, &Camera::OnWindowResize);

	/* All of the camera descriptor sets use set ID 0. */
	offsetSp1 = Add(0, renderpass.GetSubpass(0).GetSetLayout(0));
	offsetSp2 = Add(1, renderpass.GetSubpass(1).GetSetLayout(0));
	offsetSp3 = Add(2, renderpass.GetSubpass(2).GetSetLayout(0));
	offsetSp4 = Add(3, renderpass.GetSubpass(3).GetSetLayout(0));
}

Pu::Camera::Camera(Camera && value)
	: DescriptorSetGroup(std::move(value)), pos(value.pos), window(value.window),
	view(value.view), proj(value.proj), iproj(value.iproj), iview(value.iview),
	exposure(value.exposure), brightness(value.brightness), contrast(value.contrast),
	wndSize(value.wndSize), viewDirty(value.viewDirty), offsetSp1(value.offsetSp1),
	offsetSp2(value.offsetSp2), offsetSp3(value.offsetSp3), offsetSp4(value.offsetSp4)
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
		exposure = other.exposure;
		brightness = other.brightness;
		contrast = other.contrast;
		wndSize = other.wndSize;
		window = other.window;
		viewDirty = other.viewDirty;
		offsetSp1 = other.offsetSp1;
		offsetSp2 = other.offsetSp2;
		offsetSp3 = other.offsetSp3;
		offsetSp4 = other.offsetSp4;

		window->OnSizeChanged.Add(*this, &Camera::OnWindowResize);
	}

	return *this;
}

Pu::Vector3 Pu::Camera::ScreenToWorldRay(Vector2 v) const
{
	/* Convert the screen coordinates to normalized device coordinates. */
	const Vector4 ndc = Vector4(ToNDC(v), 0.0f, 0.0f);

	/* Convert to eye space and perform perspective division. */
	Vector4 eye = iproj * ndc;
	eye /= eye.W;

	/* Convert to world space and return the normalized direction from the position on the near plane to the camera. */
	const Vector4 world = GetInverseView() * eye;
	return normalize(world.XYZ - pos);
}

Pu::Vector3 Pu::Camera::ScreenToWorld(Vector2 v, float z) const
{
	/* Convert from screen coordinates to normalized device coordinates. */
	const Vector4 ndc = Vector4(ToNDC(v), z, 1.0f);

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

void Pu::Camera::SetView(const Matrix & value)
{
	view = value;
	viewDirty = true;
}

void Pu::Camera::SetProjection(const Matrix & value)
{
	proj = value;
	iproj = proj.GetInverse();
}

void Pu::Camera::Stage(DescriptorPool&, byte * dest)
{
	/* Stage the view and projection matrix for the first subpass. */
	Copy(dest + offsetSp1, &proj);
	Copy(dest + offsetSp1 + sizeof(Matrix), &view);

	/* Stage the inverse view, projection, and the camera position to the second subpass. */
	Copy(dest + offsetSp2, &iproj);
	Copy(dest + offsetSp2 + sizeof(Matrix), &GetInverseView());
	Copy(dest + offsetSp2 + (sizeof(Matrix) << 1), &pos);

	/* Stage the inverse projection matrix for the third subpass. */
	Copy(dest + offsetSp3, &iproj);
	Copy(dest + offsetSp3 + sizeof(Matrix), &GetInverseView());

	/* Stage the exposure, brightness and contrast to the fourth and final subpass. */
	Copy(dest + offsetSp4, &exposure);
	Copy(dest + offsetSp4 + sizeof(float), &brightness);
	Copy(dest + offsetSp4 + sizeof(Vector2), &contrast);
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