#pragma once
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Core/Threading/Tasks/Task.h"
#include "FieldInfo.h"

namespace Pu
{
	class SPIRVReader;

	/* Defines an object used to load and use shader modules as subpasses. */
	class Subpass
	{
	public:
		/* Defines a way to load a subpass. */
		class LoadTask
			: public Task
		{
		public:
			/* Initializes a new instance of the render pass load task. */
			LoadTask(_Out_ Subpass &result, _In_ const string &path);
			LoadTask(_In_ const LoadTask&) = delete;

			_Check_return_ LoadTask& operator =(_In_ const LoadTask&) = delete;

			/* Loads the subpass. */
			_Check_return_ virtual Result Execute(void) override;

		private:
			Subpass &result;
			string path;
		};

		/* Initializes an empty instance of a subpass. */
		Subpass(_In_ LogicalDevice &device);
		/* Creates a new shader module from a specified file. */
		Subpass(_In_ LogicalDevice &device, _In_ const string &path);
		Subpass(_In_ const Subpass&) = delete;
		/* Move constructor. */
		Subpass(_In_ Subpass &&value);
		/* Destroys the subpass. */
		~Subpass(void)
		{
			Destroy();
		}

		_Check_return_ Subpass& operator =(_In_ const Subpass&) = delete;
		/* Move assignment. */
		_Check_return_ Subpass& operator =(_In_ Subpass &&other);

		/* Gets the type (or stage) of this shader module. */
		_Check_return_ inline ShaderStageFlag GetType(void) const
		{
			return stage;
		}

		/* Gets whether the subpass has been loaded. */
		_Check_return_ inline bool IsLoaded(void) const
		{
			return loaded.load();
		}

		/* Gets the amount of field defined by the shader module. */
		_Check_return_ inline size_t GetFieldCount(void) const
		{
			return fields.size();
		}

		/* Gets the information for the field at the specified index. */
		_Check_return_ inline const FieldInfo& Getfield(_In_ size_t idx) const
		{
			return fields.size() > idx ? fields[idx] : invalid;
		}

		/* Gets the information for the specified field. */
		_Check_return_ const FieldInfo& GetField(_In_ const string &name) const;

	private:
		const static FieldInfo invalid;

		LogicalDevice &parent;
		ShaderModuleHndl hndl;
		ShaderStageFlag stage;
		vector<FieldInfo> fields;
		std::atomic_bool loaded;

		std::map<spv::Id, string> names;
		std::map<spv::Id, spv::Id> typedefs;
		std::map<spv::Id, FieldTypes> types;
		vector<std::tuple<spv::Id, spv::Id, spv::StorageClass>> variables;

		void Load(const string &path);
		void Create(const string &path);
		void SetFieldInfo(void);
		void HandleModule(SPIRVReader &reader, spv::Op opCode, size_t);
		void HandleName(SPIRVReader &reader);
		void HandleType(SPIRVReader &reader);
		void HandleInt(SPIRVReader &reader);
		void HandleFloat(SPIRVReader &reader);
		void HandleVector(SPIRVReader &reader);
		void HandleMatrix(SPIRVReader &reader);
		void HandleVariable(SPIRVReader &reader);
		void SetStage(const string &ext);
		void Destroy(void);
	};
}