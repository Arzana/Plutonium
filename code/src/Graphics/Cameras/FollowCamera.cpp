#include "Graphics/Cameras/FollowCamera.h"
#include "Core/Math/Interpolation.h"

Pu::FollowCamera::FollowCamera(const NativeWindow & wnd, DescriptorPool & pool, const Renderpass & renderpass)
	: FpsCamera(wnd, pool, renderpass), Distance(2.0f), Height(2.0f), 
	Speed(1.0f), MinDamping(2.0f), MaxDamping(20.0f), LookAt(true)
{}

void Pu::FollowCamera::Update(float dt)
{
	if (target)
	{
		/* Calculate the desired offset for the camera. */
		const Vector3 translation = target->GetTranslation();
		const Vector3 offset = target->GetOrientation() * Vector3(0.0f, Height, -Distance);

		/* Apply some damping to make the camera movement smoother. */
		const Vector3 desiredPosition = translation + offset;
		const float d = dist(GetPosition(), translation);
		SetPosition(damp(GetPosition(), desiredPosition, max(MinDamping, MaxDamping - d) * Speed, dt));

		/* Update the view matrix to always have the target in the center of the viewing plane. */
		if (LookAt) SetView(Matrix::CreateLookAt(GetPosition(), translation, Orientation * Vector3::Up()));
	}

	if (!LookAt)
	{
		/* We still need to update the view matrix incase the orientation has changed. */
		SetView(Matrix::CreateLookIn(GetPosition(), Orientation * Vector3::Forward(), Orientation * Vector3::Up()));
	}
}