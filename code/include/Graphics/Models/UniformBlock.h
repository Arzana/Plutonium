#pragma once
#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

namespace Pu
{
	/* Defines a block of undefined uniforms in a shader. */
	class UniformBlock
		: public DescriptorSet
	{
	public:
		UniformBlock(_In_ const UniformBlock&) = delete;
		/* Move constructor. */
		UniformBlock(_In_ UniformBlock &&value);
		/* Releases the resources allocated by the uniform block. */
		virtual ~UniformBlock(void)
		{
			Destroy();
		}

		_Check_return_ UniformBlock& operator =(_In_ const UniformBlock&) = delete;
		/* Move assignment. */
		_Check_return_ UniformBlock& operator =(_In_ UniformBlock &&other);

		/* Updates the uniform block if needed. */
		void Update(_In_ CommandBuffer &cmdBuffer);

	protected:
		/* Specifies whether the uniforms need to be updated on the GPU. */
		bool IsDirty;

		/* Initializes a new instance of a uniform block. */
		UniformBlock(_In_ LogicalDevice &device, _In_ const GraphicsPipeline &pipeline, _In_ std::initializer_list<string> uniforms);

		/* Loads the specified staging buffer with the new GPU data. */
		virtual void Stage(_In_ byte *destination) = 0;

	private:
		DynamicBuffer *target;
		vector<const Uniform*> uniforms;
		bool firstUpdate;

		uint32 CheckAndGetSet(const GraphicsPipeline &pipeline, std::initializer_list<string> uniforms);
		void Destroy(void);
	};
}