#pragma once
#include "Core/Math/Matrix.h"
#include "Graphics/Models/UniformBlock.h"

namespace Pu
{
	/* Defines the basic camera properies using in a shader. */
	class CameraUniformBlock
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a camera uniform block. */
		CameraUniformBlock(_In_ DescriptorPool &pool);
		CameraUniformBlock(_In_ const CameraUniformBlock&) = delete;
		/* Move constructor. */
		CameraUniformBlock(_In_ CameraUniformBlock &&value) = default;

		_Check_return_ CameraUniformBlock& operator =(_In_ const CameraUniformBlock&) = delete;
		/* Move assignment. */
		_Check_return_ CameraUniformBlock& operator =(_In_ CameraUniformBlock &&other) = default;

		/* Sets the projection and inverse projection matrix. */
		void SetProjection(_In_ const Matrix &value);
		/* Sets the view and inverse view matrix. */
		void SetView(_In_ const Matrix &value);
		
		/* Sets the position of the camera. */
		inline void SetPosition(_In_ Vector3 value)
		{
			pos = value;
			IsDirty = true;
		}

		/* Sets the exposure of this camera. */
		inline void SetExposure(_In_ float value)
		{
			exposure = value;
			IsDirty = true;
		}

		/* Sets the brightness of this camera. */
		inline void SetBrightness(_In_ float value)
		{
			brightness = value;
			IsDirty = true;
		}

		/* Sets the contrast of the camera. */
		inline void SetContrast(_In_ float value)
		{
			contrast = value;
			IsDirty = true;
		}

	protected:
		/* Stages the contents of the uniform block to the GPU. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix proj, view, iproj, iview;
		Vector3 pos;
		float exposure, brightness, contrast;
		size_t binding1, binding2;
	};
}