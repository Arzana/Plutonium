#pragma once
#include "Renderpass.h"

namespace Pu
{
	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(_In_ LogicalDevice &device, _In_ const Renderpass &renderpass);
		GraphicsPipeline(_In_ const GraphicsPipeline&) = delete;
		~GraphicsPipeline(void)
		{
			Destroy();
		}

		_Check_return_ GraphicsPipeline& operator =(_In_ const GraphicsPipeline&) = delete;

#pragma warning(push)
#pragma warning(disable:4458)
		inline void SetViewport(_In_ const Viewport &viewport)
		{
			SetViewport(viewport, viewport.GetScissor());
		}

		inline void SetViewport(_In_ const Viewport &viewport, _In_ Rect2D scissor)
		{
			this->viewport = viewport;
			this->scissor = scissor;
		}
#pragma warning(pop)

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

		LogicalDevice &parent;
		const Renderpass &renderpass;
		PipelineHndl hndl;
		PipelineLayoutHndl layoutHndl;

		Viewport viewport;
		Rect2D scissor;

		void Destroy(void);
	};
}