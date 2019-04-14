#pragma once
#include "Renderpass.h"
#include "Graphics/Vulkan/DescriptorPool.h"

namespace Pu
{
	/* Defines the enviroment for a single render pass. */
	class GraphicsPipeline
	{
	public:
		/* Occurs after the graphics pipeline has been initialized. */
		EventBus<GraphicsPipeline> PostInitialize;

		/* Initializes an empty instance of a graphics pipeline. */
		GraphicsPipeline(_In_ LogicalDevice &device, _In_ size_t maxSets);
		/* Initializes a new instance of a graphics pipeline for the specified render pass. */
		GraphicsPipeline(_In_ LogicalDevice &device, _In_ const Renderpass &renderpass, _In_ size_t maxSets);
		GraphicsPipeline(_In_ const GraphicsPipeline&) = delete;
		/* Move constructor. */
		GraphicsPipeline(_In_ GraphicsPipeline &&value);
		/* Destroys the graphics pipeline. */
		~GraphicsPipeline(void)
		{
			Destroy();
		}

		_Check_return_ GraphicsPipeline& operator =(_In_ const GraphicsPipeline&) = delete;
		/* Move assignment. */
		_Check_return_ GraphicsPipeline& operator =(_In_ GraphicsPipeline &&other);

		/* Gets the renderpass used to in this graphics pipeline. */
		_Check_return_ inline const Renderpass& GetRenderpass(void) const
		{
			return *renderpass;
		}

		/* Gets whether the graphics pipeline is ready for use. */
		_Check_return_ inline bool IsUsable(void) const 
		{
			return hndl;
		}

		/* Gets the pool from which descriptors can be made. */
		_Check_return_ inline const DescriptorPool& GetDescriptorPool(void) const
		{
			if (!pool) Log::Fatal("Cannot get descriptor pool from graphics pipeline that is not finalized!");
			return *pool;
		}

		/* Sets the cull mode to use. */
		inline void SetCullMode(_In_ CullModeFlag mode)
		{
			rasterizer.CullMode = mode;
		}

		/* Sets whether the depth and stencil tests are enabled. */
		inline void SetDepthStencilMode(_In_ bool depthTestEnabled, _In_ bool depthWriteEnbled, _In_ bool stencilEnabled)
		{
			depthStencil.DepthTestEnable = depthTestEnabled;
			depthStencil.DepthWriteEnable = depthWriteEnbled;
			depthStencil.StencilTestEnable = stencilEnabled;
		}

		/* Sets which depth compare operation to use for depth testing. */
		inline void SetDepthCompare(_In_ CompareOp operation)
		{
			depthStencil.DepthCompareOp = operation;
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
			this->vp = viewport;
			this->scissor = scissor;
		}
#pragma warning(pop)

		/* Overrides the default topology (TriangleList) to the specified value. */
		inline void SetTopology(_In_ PrimitiveTopology topology)
		{
			inputAssembly.Topology = topology;
		}

		/* Adds a vertex input binding to the graphics pipeline. */
		template <typename vertex_t>
		void AddVertexBinding(_In_ uint32 binding, _In_opt_ VertexInputRate inputRate = VertexInputRate::Vertex)
		{
			AddVertexBinding(binding, sizeof(vertex_t), inputRate);
		}

		/* Gets the blending state for a specific color blend attachment. */
		_Check_return_ PipelineColorBlendAttachmentState& GetBlendStateFor(_In_ const string &name);
		/* Adds a vertex input binding to the graphics pipeline. */
		void AddVertexBinding(_In_ uint32 binding, _In_ uint32 stride, _In_opt_ VertexInputRate inputRate = VertexInputRate::Vertex);
		/* Finalizes the graphics pipeline, no changes are allowed to be made after this is called. */
		void Finalize(void);

	private:
		friend class CommandBuffer;
		friend class DescriptorPool;
		friend class DescriptorSet;
		friend class AssetLoader;

		class LoadTask
			: public Task
		{
		public:
			LoadTask(GraphicsPipeline &pipelineResult, Renderpass &passResult, const vector<std::tuple<size_t, wstring>> &toLoad);
			LoadTask(const LoadTask&) = delete;

			LoadTask& operator =(const LoadTask&) = delete;

			virtual Result Execute(void) override;
			virtual Result Continue(void) override;

		private:
			GraphicsPipeline &result;
			Renderpass &renderPass;
			Renderpass::LoadTask *child;
		};

		PipelineVertexInputStateCreateInfo vertexInput;
		PipelineInputAssemblyStateCreateInfo inputAssembly;
		PipelineTessellationStateCreateInfo tessellation;
		PipelineViewportStateCreateInfo display;
		PipelineRasterizationStateCreateInfo rasterizer;
		PipelineMultisampleStateCreateInfo multisample;
		PipelineDepthStencilStateCreateInfo depthStencil;
		PipelineColorBlendStateCreateInfo colorBlend;
		PipelineDynamicStateCreateInfo dynamicState;

		vector<PipelineColorBlendAttachmentState> colorBlendAttachments;
		vector<VertexInputBindingDescription> bindingDescriptions;

		LogicalDevice &parent;
		const Renderpass *renderpass;
		const DescriptorPool *pool;
		PipelineHndl hndl;
		PipelineLayoutHndl layoutHndl;
		vector<DescriptorSetLayoutHndl> descriptorSets;
		Viewport vp;
		Rect2D scissor;
		size_t maxSets;

		void FinalizeLayout(void);
		void Initialize(void);
		void Destroy(void);
	};
}