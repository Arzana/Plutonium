#pragma once
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Models/UniformBlock.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	/* Defines the uniform block used to render text. */
	class ConstTextUniformBlock
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a text uniform block. */
		ConstTextUniformBlock(_In_ LogicalDevice &device, _In_ const GraphicsPipeline &pipeline);
		ConstTextUniformBlock(_In_ const ConstTextUniformBlock&) = delete;
		/* Move constructor. */
		ConstTextUniformBlock(_In_ ConstTextUniformBlock &&value);

		_Check_return_ ConstTextUniformBlock& operator =(_In_ const ConstTextUniformBlock&) = delete;
		_Check_return_ ConstTextUniformBlock& operator =(_In_ ConstTextUniformBlock&&) = delete;

		/* Sets the projection matrix used by the text renderer. */
		void SetProjection(_In_ const Matrix &matrix);
		/* Sets the font atlas. */
		void SetAtlas(_In_ const Texture2D &texture);

	protected:
		/* Stages the color to the uniform buffer. */
		virtual void Stage(byte *dest) override;

	private:
		const Uniform &uniTex;
		Matrix proj;
	};
}