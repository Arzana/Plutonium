#pragma once
#include "ShaderProgram.h"

namespace Pu
{
	/* Defines a single instance of a graphics subpass. */
	class Subpass
		: public ShaderProgram
	{
	public:
		/* Initializes an empty instance of a subpass. */
		Subpass(void) = default;
		/* Initializes a new instance of a subpass from specific shader modules. */
		Subpass(_In_ LogicalDevice &device, _In_ const vector<Shader*> &shaderModules);
		Subpass(_In_ const Subpass&) = delete;
		/* Move constructor. */
		Subpass(_In_ Subpass&&) = default;

		_Check_return_ Subpass& operator =(_In_ const Subpass&) = delete;
		/* Move assignment. */
		_Check_return_ Subpass& operator =(_In_ Subpass&&) = default;

		/* Adds a dependency with access flag None for this subpass on the previous subpass, indicating no resource transition. */
		inline void SetNoDependency(_In_ PipelineStageFlags srcStage, _In_ PipelineStageFlags dstStage, _In_opt_ DependencyFlags flags = DependencyFlags::None)
		{
			AddDependency(SubpassNotSet, srcStage, dstStage, AccessFlags::None, AccessFlags::None, flags);
		}

		/* Adds a specific dependency for this subpass on the previous subpass. */
		inline void SetDependency(_In_ PipelineStageFlags srcStage, _In_ PipelineStageFlags dstStage, _In_ AccessFlags srcAccess, _In_ AccessFlags dstAccess, _In_opt_ DependencyFlags flags = DependencyFlags::None)
		{
			AddDependency(SubpassNotSet, srcStage, dstStage, srcAccess, dstAccess, flags);
		}

		/* Adds a specific dependency on a specified subpass for the this subpass. */
		void AddDependency(_In_ uint32 srcSubpass, _In_ PipelineStageFlags srcStage, _In_ PipelineStageFlags dstStage, _In_ AccessFlags srcAccess, _In_ AccessFlags dstAccess, _In_opt_ DependencyFlags flags = DependencyFlags::None);
		/* Adds a depth/stencil buffer to the subpass. */
		_Check_return_ Output& AddDepthStencil(void);
		/* Adds a clone of a depth/stencil buffer to the subpass. */
		void CloneDepthStencil(_In_ uint32 referenceIndex);
		/* Marks the specified shader output as a clone of an already used output. */
		void CloneOutput(_In_ const string &name, _In_ uint32 referenceIndex);

	private:
		friend class Renderpass;

		constexpr static uint32 SubpassNotSet = SubpassExternal - 1;
		static FieldInfo dsInfo;

		Output *ds;
		vector<SubpassDependency> dependencies;
	};
}