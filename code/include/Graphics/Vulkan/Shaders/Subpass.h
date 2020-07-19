#pragma once
#include "Shader.h"
#include "Output.h"
#include "Attribute.h"
#include "PushConstant.h"
#include "DescriptorSetLayout.h"

namespace Pu
{
	/* Defines a single instance of a graphics subpass. */
	class Subpass
	{
	public:
		/* Initializes an empty instance of a subpass. */
		Subpass(void);
		/* Initializes a new instance of a subpass from specific shader modules. */
		Subpass(_In_ LogicalDevice &device, _In_ std::initializer_list<Shader*> shaderModules);
		/* Initializes a new instance of a subpass from specific shader modules. */
		Subpass(_In_ LogicalDevice &device, _In_ const vector<Shader*> &shaderModules);
		Subpass(_In_ const Subpass&) = delete;
		/* Move constructor. */
		Subpass(_In_ Subpass&&) = default;

		_Check_return_ Subpass& operator =(_In_ const Subpass&) = delete;
		/* Move assignment. */
		_Check_return_ Subpass& operator =(_In_ Subpass&&) = default;

		/* Gets a specific shader in this subpass. */
		_Check_return_ inline Shader& operator [](_In_ size_t idx)
		{
			return *shaders.at(idx);
		}

		/* Gets a specific shader in this subpass. */
		_Check_return_ inline const Shader& operator [](_In_ size_t idx) const
		{
			return *shaders.at(idx);
		}

		/* Adds a dependency with access flag None for this subpass on the previous subpass, indicating no resource transition. */
		inline void SetNoDependency(_In_ PipelineStageFlag srcStage, _In_ PipelineStageFlag dstStage, _In_opt_ DependencyFlag flags = DependencyFlag::None)
		{
			AddDependency(SubpassNotSet, srcStage, dstStage, AccessFlag::None, AccessFlag::None, flags);
		}

		/* Adds a specific dependency for this subpass on the previous subpass. */
		inline void SetDependency(_In_ PipelineStageFlag srcStage, _In_ PipelineStageFlag dstStage, _In_ AccessFlag srcAccess, _In_ AccessFlag dstAccess, _In_opt_ DependencyFlag flags = DependencyFlag::None)
		{
			AddDependency(SubpassNotSet, srcStage, dstStage, srcAccess, dstAccess, flags);
		}

		/* Adds a specific dependency on a specified subpass for the this subpass. */
		void AddDependency(_In_ uint32 srcSubpass, _In_ PipelineStageFlag srcStage, _In_ PipelineStageFlag dstStage, _In_ AccessFlag srcAccess, _In_ AccessFlag dstAccess, _In_opt_ DependencyFlag flags = DependencyFlag::None);
		/* Adds a depth/stencil buffer to the subpass. */
		_Check_return_ Output& AddDepthStencil(void);
		/* Adds a clone of a depth/stencil buffer to the subpass. */
		void CloneDepthStencil(_In_ uint32 referenceIndex);
		/* Gets the specified shader output. */
		_Check_return_ Output& GetOutput(_In_ const string &name);
		/* Gets the specified shader output. */
		_Check_return_ const Output& GetOutput(_In_ const string &name) const;
		/* Marks the specified shader output as a clone of an already used output. */
		void CloneOutput(_In_ const string &name, _In_ uint32 referenceIndex);
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

		/* Gets the descriptor set layout for the specified set. */
		_Check_return_ inline const DescriptorSetLayout& GetSetLayout(_In_ uint32 set) const
		{
			return setLayouts.at(set);
		}

	private:
		friend class Renderpass;
		friend class Pipeline;
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

		constexpr static uint32 SubpassNotSet = SubpassExternal - 1;

		bool linkSuccessfull;
		Output *ds;
		vector<Shader*> shaders;
		
		vector<Attribute> attributes;
		vector<Descriptor> descriptors;
		vector<PushConstant> pushConstants;
		vector<DescriptorSetLayout> setLayouts;
		vector<Output> outputs;
		vector<SubpassDependency> dependencies;

		void Link(LogicalDevice &device);
		void LoadFields(const PhysicalDevice &physicalDevice);
		bool CheckIO(const Shader &a, const Shader &b) const;
		bool CheckSets(void) const;
		std::map<uint32, vector<const Descriptor*>> QuerySets(void) const;
		void CreateSetLayouts(LogicalDevice &device);
	};
}