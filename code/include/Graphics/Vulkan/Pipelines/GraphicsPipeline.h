#pragma once
#include "Graphics/Vulkan/Shaders/Renderpass.h"

namespace Pu
{
	/* Defines a Vulkan graphics pipeline that can be used with a specific subpass. */
	class GraphicsPipeline
	{
	public:
		/* Initializes a new instance of a Vulkan graphics pipeline from a specific render pass. */
		GraphicsPipeline(_In_ const Renderpass &renderpass, _In_ uint32 subpass);
		GraphicsPipeline(_In_ const GraphicsPipeline&) = delete;
		/* Move constructor. */
		GraphicsPipeline(_In_ GraphicsPipeline &&value);
		/* Releases the resources allocated by the graphics pipeline. */
		~GraphicsPipeline(void)
		{
			Destroy();
		}

		_Check_return_ GraphicsPipeline& operator =(_In_ const GraphicsPipeline&) = delete;
		/* Move assignment. */
		_Check_return_ GraphicsPipeline& operator =(_In_ GraphicsPipeline &&other);

		/* Gets whether this graphics pipeline can be bound. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return hndl;
		}

		/* Adds a vertex binding to the graphics pipeline. */
		template <typename vertex_t>
		void AddVertexBinding(_In_ uint32 binding, _In_opt_ VertexInputRate rate = VertexInputRate::Vertex)
		{
			AddVertexBinding(binding, sizeof(vertex_t), rate);
		}

		/* Finalizes the graphics pipeline, creation the underlying resources. */
		void Finalize(void);
		/* Sets how the input assembler should assemble the vertices passed to this graphics pipeline. */
		void SetTopology(_In_ PrimitiveTopology topology);
		/* Sets the amount of path control points, if it's supported by the hardware. */
		_Check_return_ bool SetPathControlPoints(_In_ uint32 points);
		/* Sets the viewport of the graphics pipeline. */
		void SetViewport(_In_ const Viewport &viewport);
		/* Sets the viewport and the scissor rectangle of the graphics pipeline. */
		void SetViewport(_In_ const Viewport &viewport, _In_ Rect2D scissor);
		/* Enables depth clamping, if it's enabled by the hardware. */
		_Check_return_ bool EnableDepthClamp(void);
		/* Sets how the rasterizer should handle polygons, returns whether the mode is supported by the hardware. */
		_Check_return_ bool SetPolygonMode(_In_ PolygonMode mode);
		/* Sets which faces should be culled. */
		void SetCullMode(_In_ CullModeFlag mode);
		/* Sets how the front face is determined. */
		void SetFrontFace(_In_ FrontFace front);
		/* Enables depth bias and sets the scalar values for it, if it's supported by the hardware. */
		_Check_return_ bool EnableDepthBias(_In_ float constant, _In_ float clamp, _In_ float slope);
		/* Sets the line width for this graphics pipeline, if it's supported by the hardware. */
		_Check_return_ bool SetLineWidth(_In_ float width);
		/* Sets the amount of samples active in the subpass. */
		void SetSampleCount(_In_ SampleCountFlag samples);
		/* Enabled shading per sample instead of per fragment. */
		void EnableSampleShading(_In_ float min);
		/* Sets a color mask that's used for static coverage. */
		void SetSampleMask(_In_ SampleMask mask);
		/* Enabled the use of the fragments alpha to coverage for multisampling. */
		void EnableAlphaToCoverage(void);
		/* Hardsets a fragments alpha to one, if it's suported by the hardware. */
		_Check_return_ bool EnableAlphaToOne(void);
		/* Enables depth testing, which operation it should use and whether writing to depth is enabled as well. */
		void EnableDepthTest(_In_ bool enableWrites, _In_ CompareOp operation);
		/* Enabled a boudning region for depth values, if it's suported by the hardware. */
		_Check_return_ bool EnableDepthBoundsTest(_In_ float min, _In_ float max);
		/* Enables stencil testing and sets the variabled for it. */
		void EnableStencilTest(_In_ const StencilOpState &front, _In_ const StencilOpState &back);
		/* Sets the logical operation used for color blending, if it's supported by the hardware. */
		_Check_return_ bool SetBlendOperation(_In_ LogicOp operation);
		/* Sets an optional constant color used in some blend operations (default is black). */
		void SetConstantBlendColor(_In_ Color color);
		/* Marks a specific property of the graphics pipeline as dynamic. */
		void AddDynamicState(_In_ DynamicState state);
		/* Adds a vertex binding to the graphics pipeline. */
		void AddVertexBinding(_In_ uint32 binding, _In_ uint32 stride, _In_opt_ VertexInputRate rate = VertexInputRate::Vertex);
		/* Gets the blend state for a specific output attachent. */
		_Check_return_ PipelineColorBlendAttachmentState& GetBlendState(_In_ const string &output);

	private:
		friend class CommandBuffer;

		const Renderpass *renderpass;
		PipelineHndl hndl;
		uint32 subpass;

		PipelineVertexInputStateCreateInfo vertexInputState;
		PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		PipelineTessellationStateCreateInfo tessellationState;
		PipelineViewportStateCreateInfo viewportState;
		PipelineRasterizationStateCreateInfo rasterizationState;
		PipelineMultisampleStateCreateInfo multisampleState;
		PipelineDepthStencilStateCreateInfo depthStencilState;
		PipelineColorBlendStateCreateInfo colorBlendState;
		PipelineDynamicStateCreateInfo dynamicState;

		vector<PipelineColorBlendAttachmentState> colorBlendAttachments;
		vector<VertexInputBindingDescription> bindingDescriptions;
		vector<DynamicState> dynamicStates;
		Viewport viewport;
		Rect2D scissor;
		SampleMask sampleMask;

		const PhysicalDeviceFeatures& GetHardwareSupport(void) const;
		void Destroy(void);
	};
}