#pragma once
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines the uniform block used to pass the transforms to the light probe renderpass. */
	class LightProbeUniformBlock
		: public DescriptorSet
	{
	public:
		/* Initializes a new instance of a light probe uniform block from the specified descriptor pool. */
		LightProbeUniformBlock(_In_ DescriptorPool &pool);
		LightProbeUniformBlock(_In_ const LightProbeUniformBlock&) = delete;
		/* Move constructor. */
		LightProbeUniformBlock(_In_ LightProbeUniformBlock &&value) = default;

		_Check_return_ LightProbeUniformBlock& operator =(_In_ const LightProbeUniformBlock&) = delete;
		/* Move assignment. */
		_Check_return_ LightProbeUniformBlock& operator =(_In_ LightProbeUniformBlock &&other) = default;

		/* Sets one of the view matrices to a new value. */
		inline void SetFrustum(_In_ size_t idx, _In_ const Matrix &mtrx)
		{
			views[idx] = mtrx;
		}

	protected:
		/* Stages the buffer data for the uniform buffer. */
		virtual void Stage(_In_ byte *dest) override;

	private:
		Matrix views[6];
	};
}