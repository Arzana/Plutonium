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
		/* Overrides the default image reference of the output. */
		inline void SetReference(_In_ uint32 idx)
		{
			reference.Attachment = idx;
		}

		/* Sets the format of the output image. */
		inline void SetFormat(_In_ Format format)
		{
			description.Format = format;
		}

		/* Sets the layout of the output to a specified value. */
		inline void SetLayout(_In_ ImageLayout layout)
		{
			reference.Layout = layout;
		}

		/* Sets the layout of the output before this output is used in the renderpass. */
		inline void SetInitialLayout(_In_ ImageLayout layout)
		{
			description.InitialLayout = layout;
		}

		/* Sets the layout of the output after this output is used in the renderpass. */
		inline void SetFinalLayout(_In_ ImageLayout layout)
		{
			description.FinalLayout = layout;
		}

		/* Sets the layout of the output before, during and after it is used in the renderpass. */
		inline void SetLayouts(_In_ ImageLayout layout)
		{
			description.InitialLayout = layout;
			reference.Layout = layout;
			description.FinalLayout = layout;
		}

		/* Set the layout of the output before, during and after it is used in the renderpass. */
		inline void SetLayouts(_In_ ImageLayout initial, _In_ ImageLayout layout, _In_ ImageLayout final)
		{
			description.InitialLayout = initial;
			reference.Layout = layout;
			description.FinalLayout = final;
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

		/* Sets the color store operation of this output. */
		inline void SetStoreOperation(_In_ AttachmentStoreOp op)
		{
			description.StoreOp = op;
		}

		/* Sets the output to the specified depth buffer format. */
		inline void SetDescription(_In_ const DepthBuffer &depthBuffer)
		{
			SetDepthDescription(depthBuffer.GetFormat());
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
		void SetDepthDescription(_In_ Format format);

	private:
		friend class Subpass;
		friend class Renderpass;
		friend class GraphicsPipeline;

		bool clone;
		uint32 subpass;
		OutputUsage type;
		ClearValue clear;
		AttachmentReference reference;
		AttachmentDescription description;
		PipelineColorBlendAttachmentState attachment;

		Output(const FieldInfo &data, uint32 attachment, OutputUsage type);
	};
}