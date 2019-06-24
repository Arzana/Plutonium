#pragma once
#include "Core/Math/Matrix.h"
#include "Graphics/Models/UniformBlock.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	/* Defines the uniform block used in the debug renderer. */
	class DebugRendererUniformBlock
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a debug renderer uniform block. */
		DebugRendererUniformBlock(_In_ const Subpass &subpass, _In_ DescriptorPool &pool);
		DebugRendererUniformBlock(_In_ const DebugRendererUniformBlock&) = delete;
		/* Move constructor. */
		DebugRendererUniformBlock(_In_ DebugRendererUniformBlock &&value);

		_Check_return_ DebugRendererUniformBlock& operator =(_In_ const DebugRendererUniformBlock&) = delete;
		/* Move assignment. */
		_Check_return_ DebugRendererUniformBlock& operator =(_In_ DebugRendererUniformBlock &&other);

		/* Updates the projection matrix of the uniform block. */
		void SetProjection(_In_ const Matrix &value);
		/* Updates the model matrix of the uniform block. */
		void SetView(_In_ const Matrix &value);

	protected:
		/* Stages the projection and view matrix to the unfirom buffer. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix proj, view;
	};
}