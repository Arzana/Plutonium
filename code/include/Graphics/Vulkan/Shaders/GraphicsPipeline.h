#pragma once
#include "Renderpass.h"

namespace Pu
{
	/* Defines the enviroment for a single render pass. */
	class GraphicsPipeline
	{
	public:
		/* Defines a way to load a graphics pipeline. */
		class LoadTask
			: public Task
		{
		public:
			/* Initializes a new instance of the graphics pipeline load task. */
			LoadTask(_Out_ GraphicsPipeline &pipelineResult, _Out_ Renderpass &passResult, _In_ std::initializer_list<const char*> subpasses);
			LoadTask(_In_ const LoadTask&) = delete;

			_Check_return_ LoadTask& operator =(_In_ const LoadTask&) = delete;

			/* Loads the render pass. */
			_Check_return_ virtual Result Execute(void) override;
			/* Initializes the graphics pipeline. */
			_Check_return_ virtual Result Continue(void) override;

		private:
			GraphicsPipeline &result;
			Renderpass &renderPass;
			Renderpass::LoadTask *child;
		};

		/* Occurs after the graphics pipeline has been initialized. */
		EventBus<GraphicsPipeline, EventArgs> PostInitialize;

		/* Initializes an empty instance of a graphics pipeline. */
		GraphicsPipeline(_In_ LogicalDevice &device);
		/* Initializes a new instance of a graphics pipeline for the specified render pass. */
		GraphicsPipeline(_In_ LogicalDevice &device, _In_ const Renderpass &renderpass);
		GraphicsPipeline(_In_ const GraphicsPipeline&) = delete;
		GraphicsPipeline(_In_ GraphicsPipeline&&) = delete;
		/* Destroys the graphics pipeline. */
		~GraphicsPipeline(void);

		_Check_return_ GraphicsPipeline& operator =(_In_ const GraphicsPipeline&) = delete;
		_Check_return_ GraphicsPipeline& operator =(_In_ GraphicsPipeline&&) = delete;

		/* Gets whether the graphics pipeline has been loaded. */
		_Check_return_ inline bool IsLoaded(void) const
		{
			return loaded.load();
		}

#pragma warning(push)
#pragma warning(disable:4458)
		/* Sets the viewport parameters of the graphics pipeline. */
		inline void SetViewport(_In_ const Viewport &viewport)
		{
			SetViewport(viewport, viewport.GetScissor());
		}

		/* Sets the viewport and scissor rectangle parameters of the graphics pipeline. */
		inline void SetViewport(_In_ const Viewport &viewport, _In_ Rect2D scissor)
		{
			this->viewport = viewport;
			this->scissor = scissor;
		}
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
		const Renderpass *renderpass;
		PipelineHndl hndl;
		PipelineLayoutHndl layoutHndl;
		std::atomic_bool loaded;

		Viewport viewport;
		Rect2D scissor;

		void Initialize(void);
		void Destroy(void);
	};
}