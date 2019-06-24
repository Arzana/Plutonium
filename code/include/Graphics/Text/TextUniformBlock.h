#pragma once
#include "Core/Math/Matrix.h"
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
		TextUniformBlock(_In_ const Subpass &subpass, _In_ DescriptorPool &pool);
		TextUniformBlock(_In_ const TextUniformBlock&) = delete;
		/* Move constructor. */
		TextUniformBlock(_In_ TextUniformBlock &&value);

		_Check_return_ TextUniformBlock& operator =(_In_ const TextUniformBlock&) = delete;
		_Check_return_ TextUniformBlock& operator =(_In_ TextUniformBlock&&) = delete;

		/* Updates the color of the uniform block. */
		void SetColor(_In_ Color color);
		/* Updates the model matrix of the uniform block. */
		void SetModel(_In_ const Matrix &matrix);

		/* Gets the color of the text. */
		_Check_return_ inline Color GetColor(void) const
		{
			return Color(clr);
		}

	protected:
		/* Stages the model matrix and the color to the uniform buffer. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix model;
		Vector4 clr;
		size_t allignedOffset;
	};
}