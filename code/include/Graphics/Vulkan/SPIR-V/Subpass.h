#pragma once
#include "Graphics/Vulkan/LogicalDevice.h"

namespace Pu
{
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

		void Create(const string &path);
		void SetStage(const string &ext);
		void Destroy(void);
	};
}