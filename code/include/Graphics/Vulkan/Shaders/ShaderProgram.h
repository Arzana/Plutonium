#pragma once
#include "Shader.h"
#include "Output.h"
#include "Attribute.h"
#include "PushConstant.h"
#include "DescriptorSetLayout.h"

namespace Pu
{
	/* Defines an instance of a linked graphics or compute shader program. */
	class ShaderProgram
	{
	public:
		/* Initializes an empty instance of a shader program. */
		ShaderProgram(void);
		/* Initializes a new instance of a shader program from specific shader modules. */
		ShaderProgram(_In_ LogicalDevice &device, _In_ const vector<Shader*> &shaderModules);
		ShaderProgram(_In_ const ShaderProgram&) = delete;
		/* Move constructor. */
		ShaderProgram(_In_ ShaderProgram &&value) = default;

		_Check_return_ ShaderProgram& operator =(_In_ const ShaderProgram&) = delete;
		/* Move assignment. */
		_Check_return_ ShaderProgram& operator =(_In_ ShaderProgram &&other) = default;

		/* Gets a specific shader in this program. */
		_Check_return_ inline Shader& operator [](_In_ size_t idx)
		{
			return *shaders.at(idx);
		}

		/* Gets a specific shader in this program. */
		_Check_return_ inline const Shader& operator [](_In_ size_t idx) const
		{
			return *shaders.at(idx);
		}

		/* Gets the descriptor set layout for the specified set. */
		_Check_return_ inline const DescriptorSetLayout& GetSetLayout(_In_ uint32 set) const
		{
			return setLayouts.at(set);
		}

		/* Gets whether this shader program was created with valid arguments. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return linkSuccessfull;
		}

		/* Gets the shaders used in this shader program. */
		_Check_return_ const vector<Shader*>& GetShaders(void) const
		{
			return shaders;
		}

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

	private:
		friend class Subpass;
		friend class Pipeline;
		friend class Renderpass;
		friend class AssetLoader;
		friend class GraphicsPipeline;

		static FieldInfo defInfo;
		static Output defOutput;
		static Attribute defAttrib;
		static Descriptor defDescr;
		static PushConstant defConst;

		bool linkSuccessfull;
		vector<Shader*> shaders;

		vector<Attribute> attributes;
		vector<Descriptor> descriptors;
		vector<PushConstant> pushConstants;
		vector<DescriptorSetLayout> setLayouts;
		vector<Output> outputs;

		void Link(LogicalDevice &device);
		void LoadFields(const PhysicalDevice &physicalDevice);
		bool CheckIO(const Shader &a, const Shader &b) const;
		bool CheckSets(void) const;
		std::map<uint32, vector<const Descriptor*>> QuerySets(void) const;
		void CreateSetLayouts(LogicalDevice &device);
	};
}