#pragma once
#include "Core/Math/Matrix.h"
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Core/Events/ValueChangedEventArgs.h"
#include "Graphics/Textures/TextureCube.h"

/* Used temporarily when before switching to deferred fulltime. */
#define PU_CAMERA_USE_FORWARD

namespace Pu
{
	class NativeWindow;

	/* Defines a camera base. */
	class Camera
		: public DescriptorSet
	{
	public:
		Camera(_In_ const Camera&) = delete;
		/* Move constructor. */
		Camera(_In_ Camera &&value);
		/* Releases the camera. */
		virtual ~Camera(void)
		{
			Destroy();
		}

		_Check_return_ Camera& operator =(_In_ const Camera&) = delete;
		/* Move constructor. */
		_Check_return_ Camera& operator =(_In_ Camera &&other);

		/* Convertes the specified screen coordinate to a ray pointing from the camera's position into the world. */
		_Check_return_ Vector3 ScreenToWorldRay(_In_ Vector2 v) const;
		/* Converts the specified screen coordinate to a world space position (z is in NDC space). */
		_Check_return_ Vector3 ScreenToWorld(_In_ Vector2 v, _In_ float z) const;
		/* Gets the camera's inverse view matrix. */
		_Check_return_ const Matrix& GetInverseView(void) const;

		/* Gets the position of the camera. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return pos;
		}

		/* Gets the camera's view matrix. */
		_Check_return_ inline const Matrix& GetView(void) const
		{
			return view;
		}

		/* Gets the camera's projection matrix. */
		_Check_return_ inline const Matrix& GetProjection(void) const
		{
			return proj;
		}

		/* Gets the camera's inverse projection matrix. */
		_Check_return_ inline const Matrix& GetInverseProjection(void) const
		{
			return iproj;
		}

		/* Gets the exposure of the camera. */
		_Check_return_ inline float GetExposure(void) const
		{
			return exposure;
		}

		/* Gets the contrast of the camera. */
		_Check_return_ inline float GetContrast(void) const
		{
			return contrast;
		}

		/* Gets the brightness of the camera. */
		_Check_return_ inline float Brightness(void) const
		{
			return brightness;
		}

		/* Sets the position of the camera. */
		inline virtual void SetPosition(_In_ Vector3 position)
		{
			pos = position;
		}

		/* Adds the specified offset to the position. */
		inline void Move(_In_ Vector3 offset)
		{
			SetPosition(pos + offset);
		}

		/* Adds the specified offset to the position. */
		inline void Move(_In_ float x, _In_ float y, _In_ float z)
		{
			SetPosition(pos + Vector3(x, y, z));
		}

		/* Sets the exposure of this camera. */
		inline void SetExposure(_In_ float value)
		{
			exposure = value;
		}

		/* Sets the brightness of this camera. */
		inline void SetBrightness(_In_ float value)
		{
			brightness = value;
		}

		/* Sets the contrast of the camera. */
		inline void SetContrast(_In_ float value)
		{
			contrast = value;
		}

#ifdef PU_CAMERA_USE_FORWARD
		inline void SetEnvironment(const TextureCube &probe)
		{
			Write(*envMap, probe);
		}
#endif

	protected:
		/* Initializes a new instance of a camera. */
		Camera(_In_ const NativeWindow &wnd, _In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout);

		/* Updates the camera. */
		virtual void Update(_In_ float /*dt*/) {};
		/* Sets the view matrix. */
		void SetView(_In_ const Matrix &value);
		/* Sets the projection matrix. */
		void SetProjection(_In_ const Matrix &value);
		/* Stages the contents of the uniform block to the GPU. */
		virtual void Stage(byte *dest) override;
		/* Occurs when the native window changes it's size. */
		virtual void OnWindowResize(const NativeWindow&, ValueChangedEventArgs<Vector2>);

	private:
		Vector3 pos;
		Matrix view, proj, iproj;
		float exposure, brightness, contrast;
		Vector2 wndSize;
		const NativeWindow *window;

#ifndef PU_CAMERA_USE_FORWARD
		size_t binding1, binding2;
#else
		size_t binding1;
		const Descriptor *envMap;
#endif

		mutable Matrix iview;
		mutable bool viewDirty;

		void Destroy(void);
		Vector2 ToNDC(Vector2 v) const;
	};
}