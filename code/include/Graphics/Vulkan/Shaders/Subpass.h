#pragma once
#include "Shader.h"
#include "Output.h"
#include "Attribute.h"
#include "Descriptor.h"

namespace Pu
{
	/* Defines a single instance of a graphics subpass. */
	class Subpass
	{
	public:
		/* Initializes an empty instance of a subpass. */
		Subpass(void);
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

		/* Sets the dependency information for this subpass. */
		void SetInformation(_In_ PipelineStageFlag stage, _In_ AccessFlag access);
		/* Sets the dependency information for the previous subpass. */
		void SetDependency(_In_ PipelineStageFlag stage, _In_ AccessFlag access);
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
		friend class GraphicsPipeline;
		friend class DescriptorPool;
		friend class AssetLoader;

		static FieldInfo dsInfo;
		static FieldInfo defInfo;
		static Output defOutput;
		static Attribute defAttrib;
		static Descriptor defDescr;

		bool linkSuccessfull;
		Output *ds;
		vector<Shader*> shaders;
		
		vector<Attribute> attributes;
		vector<Descriptor> descriptors;
		vector<Output> outputs;

		/* src and dest subpasses are set by the renderpass. */
		SubpassDependency dependency;
		bool dependencyUsed;

		void Link(const PhysicalDevice &physicalDevice);
		void LoadFields(const PhysicalDevice &physicalDevice);
		bool CheckIO(const Shader &a, const Shader &b);
	};
}