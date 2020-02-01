#pragma once
#include "Shader.h"
#include "Output.h"
#include "Attribute.h"
#include "Descriptor.h"
#include "PushConstant.h"

namespace Pu
{
	/* Defines a single instance of a graphics subpass. */
	class Subpass
	{
	public:
		/* Initializes an empty instance of a subpass. */
		Subpass(void);
		/* Initializes a new instance of a subpass from specific shader modules. */
		Subpass(_In_ const PhysicalDevice &physicalDevice, _In_ std::initializer_list<Shader*> shaderModules);
		/* Initializes a new instance of a subpass from specific shader modules. */
		Subpass(_In_ const PhysicalDevice &physicalDevice, _In_ const vector<Shader*> &shaderModules);
		/* Copy constructor. */
		Subpass(_In_ const Subpass&) = default;
		/* Move constructor. */
		Subpass(_In_ Subpass&&) = default;

		/* Copy assignment. */
		_Check_return_ Subpass& operator =(_In_ const Subpass&) = default;
		/* Move assignment. */
		_Check_return_ Subpass& operator =(_In_ Subpass&&) = default;

		/* Sets the dependency information for the this subpass. */
		void SetDependency(_In_ PipelineStageFlag srcStage, _In_ PipelineStageFlag dstStage, _In_ AccessFlag srcAccess, _In_ AccessFlag dstAccess, _In_opt_ DependencyFlag flags = DependencyFlag::None);
		/* Adds a depth/stencil buffer to the subpass. */
		_Check_return_ Output& AddDepthStencil(void);
		/* Gets the specified shader output. */
		_Check_return_ Output& GetOutput(_In_ const string &name);
		/* Gets the specified shader output. */
		_Check_return_ const Output& GetOutput(_In_ const string &name) const;
		/* Gets the specified shader input attribute. */
		_Check_return_ Attribute& GetAttribute(_In_ const string &name);
		/* Gets the specified shader input attribute. */
		_Check_return_ const Attribute& GetAttribute(_In_ const string &name) const;
		/* Gets the specified shader input descriptor. */
		_Check_return_ Descriptor& GetDescriptor(_In_ const string &name);
		/* Gets the specified shader input descriptor. */
		_Check_return_ const Descriptor& GetDescriptor(_In_ const string &name) const;
		/* Gets the specified push constant. */
		_Check_return_ PushConstant& GetPushConstant(_In_ const string &name);
		/* Gets the specified push constant. */
		_Check_return_ const PushConstant& GetPushConstant(_In_ const string &name) const;

		/* Gets whether this subpass was created with valid arguments. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return linkSuccessfull;
		}

		/* Gets the shaders used in this subpass. */
		_Check_return_ const vector<Shader*>& GetShaders(void) const
		{
			return shaders;
		}

	private:
		friend class Renderpass;
		friend class PipelineLayout;
		friend class GraphicsPipeline;
		friend class DescriptorPool;
		friend class UniformBlock;
		friend class AssetLoader;

		static FieldInfo dsInfo;
		static FieldInfo defInfo;
		static Output defOutput;
		static Attribute defAttrib;
		static Descriptor defDescr;
		static PushConstant defConst;

		bool linkSuccessfull;
		Output *ds;
		vector<Shader*> shaders;
		
		vector<Attribute> attributes;
		vector<Descriptor> descriptors;
		vector<PushConstant> pushConstants;
		vector<Output> outputs;

		/* src and dest subpasses are set by the renderpass. */
		SubpassDependency dependency;
		bool dependencyUsed;

		void Link(const PhysicalDevice &physicalDevice);
		void LoadFields(const PhysicalDevice &physicalDevice);
		bool CheckIO(const Shader &a, const Shader &b);
	};
}