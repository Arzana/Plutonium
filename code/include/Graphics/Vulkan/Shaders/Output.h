#pragma once
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Textures/DepthBuffer.h"
#include "Field.h"
#include "OutputUsage.h"

namespace Pu
{
	/* Specifies information about a shader output field. */
	class Output
		: public Field
	{
	public:
		/* Sets the layout of the output to a specified value. */
		inline void SetLayout(_In_ ImageLayout layout)
		{
			reference.Layout = layout;
		}

		/* Sets the format of the output image. */
		inline void SetFormat(_In_ Format format)
		{
			description.Format = format;
		}

		/* Sets the layout of the output before this output is used in the render pass. */
		inline void SetInitialLayout(_In_ ImageLayout layout)
		{
			description.InitialLayout = layout;
		}

		/* Sets the layout of the output after this output is used in the render pass. */
		inline void SetFinalLayout(_In_ ImageLayout layout)
		{
			description.FinalLayout = layout;
		}

		/* Sets how this output should be cleared before use. */
		inline void SetClearValue(_In_ ClearValue value)
		{
			clear = value;
		}

		/* Sets the color load operation of this output. */
		inline void SetLoadOperation(_In_ AttachmentLoadOp op)
		{
			description.LoadOp = op;
		}

		/* Sets how the output field should be handled. */
		void SetUsage(_In_ OutputUsage usage);
		/* Enables and sets the blend operations on color. */
		void SetColorBlending(_In_ BlendFactor srcFactor, _In_ BlendOp op, _In_ BlendFactor dstFactor);
		/* Enables and sets the blend operations on alpha. */
		void SetAlphaBlending(_In_ BlendFactor srcFactor, _In_ BlendOp op, _In_ BlendFactor dstFactor);
		/* Sets the output to the specified swapchain format. */
		void SetDescription(_In_ const Swapchain &swapchain);
		/* Sets the output to the specified depth buffer format. */
		void SetDescription(_In_ const DepthBuffer &depthBuffer);

	private:
		friend class Subpass;
		friend class Renderpass;
		friend class GraphicsPipeline;

		uint32 subpass;
		OutputUsage type;
		ClearValue clear;
		AttachmentReference reference;
		AttachmentDescription description;
		PipelineColorBlendAttachmentState attachment;

		Output(const FieldInfo &data, uint32 attachment, OutputUsage type);
	};
}