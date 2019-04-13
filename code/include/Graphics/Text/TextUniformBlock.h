#pragma once
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Models/UniformBlock.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	/* Defines the uniform block used to render text. */
	class TextUniformBlock
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a text uniform block. */
		TextUniformBlock(_In_ LogicalDevice &device, _In_ const GraphicsPipeline &pipeline);
		TextUniformBlock(_In_ const TextUniformBlock&) = delete;
		/* Move constructor. */
		TextUniformBlock(_In_ TextUniformBlock &&value);

		_Check_return_ TextUniformBlock& operator =(_In_ const TextUniformBlock&) = delete;
		_Check_return_ TextUniformBlock& operator =(_In_ TextUniformBlock&&) = delete;

		/* Updates the color of the uniform block. */
		void SetColor(_In_ Color color);
		/* Updates the font atlas of the uniform block. */
		void SetAtlas(_In_ const Texture2D &atlas);

	protected:
		/* Stages the color to the uniform buffer. */
		virtual void Stage(byte *dest) override;
		/* Updates the descriptors in the uniform buffer. */
		virtual void UpdateDescriptor(DescriptorSet &set, const Buffer &uniformBuffer) override;

	private:
		const Uniform &uniTex;
		const Uniform &uniClr;

		const Texture2D *texture;
		Vector4 clr;
	};
}