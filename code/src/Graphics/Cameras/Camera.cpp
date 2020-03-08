#include "Graphics/Cameras/Camera.h"
#include "Application.h"

Pu::Camera::Camera(const NativeWindow & wnd, DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, 0, layout), viewDirty(false),
	exposure(1.0f), brightness(0.0f), contrast(1.0f),
	window(&wnd)
{
	wnd.OnSizeChanged.Add(*this, &Camera::OnWindowResize);

#ifndef PU_CAMERA_USE_FORWARD
	binding1 = GetDescriptor(1, "IProjection").GetAllignedOffset(sizeof(Matrix) << 1);
	binding2 = GetDescriptor(2, "Exposure").GetAllignedOffset((sizeof(Matrix) << 2) + sizeof(Vector3));
#else
	binding1 = GetDescriptor(0, "CamPos").GetAllignedOffset(sizeof(Pu::Matrix) * 2);
	envMap = &GetDescriptor(0, "Environment");
#endif
}

Pu::Camera::Camera(Camera && value)
	: DescriptorSet(std::move(value)), pos(value.pos), window(value.window),
	view(value.view), proj(value.proj), iproj(value.iproj), iview(value.iview),
	exposure(value.exposure), brightness(value.brightness), contrast(value.contrast),
	wndSize(value.wndSize), viewDirty(value.viewDirty), binding1(value.binding1),
#ifndef PU_CAMERA_USE_FORWARD
	binding2(value.binding2)
#else
	envMap(value.envMap)
#endif
{
	window->OnSizeChanged.Add(*this, &Camera::OnWindowResize);
}

Pu::Camera & Pu::Camera::operator=(Camera && other)
{
	if (this != &other)
	{
		Destroy();
		DescriptorSet::operator=(std::move(other));

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
		binding1 = other.binding1;

#ifndef PU_CAMERA_USE_FORWARD	
		binding2 = other.binding2;
#else
		envMap = other.envMap;
#endif

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

void Pu::Camera::Stage(byte * dest)
{
	/* Binding 0 (G-Pass). */
	Copy(dest, &proj);
	Copy(dest + sizeof(Matrix), &view);

#ifndef PU_CAMERA_USE_FORWARD
	/* Binding 1 (Light-Pass). */
	Copy(dest + binding1, &iproj);
	Copy(dest + binding1 + sizeof(Matrix), &iview);
	Copy(dest + binding1 + (sizeof(Matrix) << 1), &pos);

	/* Binding 2 (Tonemap-Pass). */
	Copy(dest + binding2, &exposure);
	Copy(dest + binding2 + sizeof(float), &brightness);
	Copy(dest + binding2 + (sizeof(float) << 1), &contrast);
#else
	Copy(dest + binding1, &pos);
	Copy(dest + binding1 + sizeof(Vector3), &brightness);
	Copy(dest + binding1 + sizeof(Vector4), &contrast);
	Copy(dest + binding1 + sizeof(float) * 5, &exposure);
#endif
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