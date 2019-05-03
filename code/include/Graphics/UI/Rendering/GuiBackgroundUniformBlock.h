#pragma once
#include "Graphics/Models/UniformBlock.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	/* Defines the uniform block used for set 1 in the GUI background shader. */
	class GuiBackgroundUniformBlock
		: public UniformBlock
	{
	public:
		/* Initializes a new instance of a GUI background uniform block. */
		GuiBackgroundUniformBlock(_In_ LogicalDevice &device, _In_ const GraphicsPipeline &pipeline);
		GuiBackgroundUniformBlock(_In_ const GuiBackgroundUniformBlock&) = delete;
		/* Move constructor. */
		GuiBackgroundUniformBlock(_In_ GuiBackgroundUniformBlock &&value);

		_Check_return_ GuiBackgroundUniformBlock& operator =(_In_ const GuiBackgroundUniformBlock&) = delete;
		_Check_return_ GuiBackgroundUniformBlock& operator =(_In_ GuiBackgroundUniformBlock&&) = delete;

		/* Updates the model matrix of the uniform block. */
		void SetModel(_In_ const Matrix &value);
		/* Updates the background color mask of the uniform block. */
		void SetColor(_In_ Color value);

		/* Gets the background color used in the uniform block. */
		_Check_return_ inline Color GetColor(void) const
		{
			return Color(color);
		}

	protected:
		/* Stages the two binding for this uniform block. */
		virtual void Stage(byte *dest) override;

	private:
		Matrix model;
		Vector4 color;
		size_t allignedOffset;
	};
}