#pragma once
#include "Pipeline.h"

namespace Pu
{
	/* Defines a Vulkan compute pipeline that can be used with a specific shader. */
	class ComputePipeline
		: public Pipeline
	{
	public:
		/* Initializes a new instance of a Vulkan compute pipeline from a specific shader program. */
		ComputePipeline(_In_ const ShaderProgram &program);
		ComputePipeline(_In_ const ComputePipeline&) = delete;
		/* Move constructor. */
		ComputePipeline(_In_ ComputePipeline &&value) = default;

		_Check_return_ ComputePipeline& operator =(_In_ const ComputePipeline&) = delete;
		/* Move assignment. */
		_Check_return_ ComputePipeline& operator =(_In_ ComputePipeline &&other) = default;

		/* Gets the shader used for the compute pass. */
		_Check_return_ inline Shader& GetShader(void)
		{
			return *program->GetShaders()[0];
		}

		/* Gets the shader used for the compute pass. */
		_Check_return_ inline const Shader& GetShader(void) const
		{
			return *program->GetShaders()[0];
		}

		/* Gets the specified specialization constant. */
		_Check_return_ inline SpecializationConstant& GetSpecializationConstant(_In_ const string &name)
		{
			GetShader().GetConstant(name);
		}

		/* Finalizes the compute pipeline, creation the underlying resources. */
		void Finalize(void);

	private:
		const ShaderProgram *program;
	};
}