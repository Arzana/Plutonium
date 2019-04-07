#pragma once
#include "Core/Threading/Tasks/Task.h"
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/SPIR-V/FieldInfo.h"
#include "Graphics/Vulkan/SPIR-V/Decoration.h"
#include "Content/Asset.h"

namespace Pu
{
	class SPIRVReader;

	/* Defines an object used to load and use shader modules. */
	class Shader
		: public Asset
	{
	public:
		/* Initializes an empty instance of a Shader. */
		Shader(_In_ LogicalDevice &device);
		/* Creates a new shader module from a specified file. */
		Shader(_In_ LogicalDevice &device, _In_ const wstring &path);
		Shader(_In_ const Shader&) = delete;
		/* Move constructor. */
		Shader(_In_ Shader &&value);
		/* Destroys the Shader. */
		virtual ~Shader(void)
		{
			Destroy();
		}

		_Check_return_ Shader& operator =(_In_ const Shader&) = delete;
		/* Move assignment. */
		_Check_return_ Shader& operator =(_In_ Shader &&other);

		/* Gets the type (or stage) of this shader module. */
		_Check_return_ inline ShaderStageFlag GetType(void) const
		{
			return info.Stage;
		}

		/* Gets the name assigned to this shader module. */
		_Check_return_ inline const wstring& GetName(void) const
		{
			return name;
		}

		/* Gets the amount of field defined by the shader module. */
		_Check_return_ inline size_t GetFieldCount(void) const
		{
			return fields.size();
		}

		/* Gets the information for the field at the specified index. */
		_Check_return_ inline const FieldInfo& GetField(_In_ size_t idx) const
		{
			return fields.size() > idx ? fields[idx] : invalid;
		}

		/* Gets the information for the specified field. */
		_Check_return_ const FieldInfo& GetField(_In_ const string &name) const;

	protected:
		/* References the asset and return a self-reference. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

	private:
		friend class GraphicsPipeline;
		friend class Renderpass;

		class LoadTask
			: public Task
		{
		public:
			LoadTask(Shader &result, const wstring &path);
			LoadTask(const LoadTask&) = delete;

			LoadTask& operator =(const LoadTask&) = delete;

			virtual Result Execute(void) override;

		private:
			Shader &result;
			wstring path;
		};

		const static FieldInfo invalid;

		LogicalDevice &parent;
		PipelineShaderStageCreateInfo info;
		vector<FieldInfo> fields;
		wstring name;

		std::map<spv::Id, string> names;
		std::map<spv::Id, vector<string>> memberNames;
		std::map<spv::Id, spv::Id> typedefs;
		std::map<spv::Id, FieldType> types;
		std::map<spv::Id, vector<spv::Id>> structs;
		std::map<spv::Id, Decoration> decorations;
		vector<std::tuple<spv::Id, spv::Id, spv::StorageClass>> variables;

		void Load(const wstring &path, bool viaLoader);
		void Create(const wstring &path);
		void SetFieldInfo(void);
		void HandleVariable(spv::Id id, spv::Id typeId, spv::StorageClass storage);
		void HandleModule(SPIRVReader &reader, spv::Op opCode, size_t wordCnt);
		void HandleName(SPIRVReader &reader);
		void HandleMemberName(SPIRVReader &reader);
		void HandleDecorate(SPIRVReader &reader);
		void HandleType(SPIRVReader &reader);
		void HandleInt(SPIRVReader &reader);
		void HandleFloat(SPIRVReader &reader);
		void HandleVector(SPIRVReader &reader);
		void HandleMatrix(SPIRVReader &reader);
		void HandleStruct(SPIRVReader &reader, size_t memberCnt);
		void HandleImage(SPIRVReader &reader);
		void HandleSampledImage(SPIRVReader &reader);
		void HandleVariable(SPIRVReader &reader);
		void SetInfo(const wstring &ext);
		void Destroy(void);
	};
}