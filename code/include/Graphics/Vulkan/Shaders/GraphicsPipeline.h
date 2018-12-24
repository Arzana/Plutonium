#pragma once
#include "Renderpass.h"

namespace Pu
{
	/* Defines the enviroment for a single render pass. */
	class GraphicsPipeline
	{
	public:
		/* Initializes a new instance of a graphics pipeline for the specified render pass. */
		GraphicsPipeline(_In_ LogicalDevice &device, _In_ const Renderpass &renderpass);
		GraphicsPipeline(_In_ const GraphicsPipeline&) = delete;
		/* Destroys the graphics pipeline. */
		~GraphicsPipeline(void)
		{
			Destroy();
		}

		_Check_return_ GraphicsPipeline& operator =(_In_ const GraphicsPipeline&) = delete;

#pragma warning(push)
#pragma warning(disable:4458)
		/* Sets the viewport parameters of the graphics pipeline. */
		inline void SetViewport(_In_ const Viewport &viewport)
		{
			SetViewport(viewport, viewport.GetScissor());
		}

		/* Sets the viewport and scissor rectangle parameters of the graphics pipeline. */
		void SetViewport(_In_ const Viewport &viewport, _In_ Rect2D scissor);
#pragma warning(pop)

		/* Gets the blending state for a specific color blend attachment. */
		_Check_return_ PipelineColorBlendAttachmentState& GetBlendStateFor(_In_ const string &name);
		/* Finalizes the graphics pipeline, no changes are allowed to be made after this is called. */
		void Finalize(void);

	private:
		friend class CommandBuffer;

		PipelineVertexInputStateCreateInfo *vertexInput;
		PipelineInputAssemblyStateCreateInfo *inputAssembly;
		PipelineTessellationStateCreateInfo *tessellation;
		PipelineViewportStateCreateInfo *display;
		PipelineRasterizationStateCreateInfo *rasterizer;
		PipelineMultisampleStateCreateInfo *multisample;
		PipelineDepthStencilStateCreateInfo *depthStencil;
		PipelineColorBlendStateCreateInfo *colorBlend;
		PipelineDynamicStateCreateInfo *dynamicState;

		vector<PipelineColorBlendAttachmentState> colorBlendAttachments;

		LogicalDevice &parent;
		const Renderpass &renderpass;
		PipelineHndl hndl;
		PipelineLayoutHndl layoutHndl;

		Viewport viewport;
		Rect2D scissor;

		void Destroy(void);
	};
}