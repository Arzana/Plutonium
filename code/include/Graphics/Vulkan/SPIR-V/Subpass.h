#pragma once
#include "SPIRV.h"
#include "Graphics/Vulkan/LogicalDevice.h"
#include "FieldTypes.h"

namespace Pu
{
	class SPIRVReader;

	/* Defines a reflection object for shader modules. */
	class Subpass
	{
	public:
		/* Creates a new shader module from a specified file. */
		Subpass(_In_ LogicalDevice &device, _In_ const char *path);
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

	private:
		LogicalDevice &parent;
		ShaderModuleHndl hndl;
		ShaderStageFlag stage;
		std::map<spv::Id, string> names;
		std::map<spv::Id, spv::Id> typedefs;
		std::map<spv::Id, FieldTypes> types;
		std::vector<std::tuple<spv::Id, spv::Id, spv::StorageClass>> variables;

		void Create(const string &path);
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