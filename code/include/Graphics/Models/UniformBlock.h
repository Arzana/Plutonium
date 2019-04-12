#pragma once
#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

namespace Pu
{
	/* Defines a block of undefined uniforms in a shader. */
	class UniformBlock
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

		/* Gets the object that describes the uniform buffer. */
		_Check_return_ inline const DescriptorSet& GetDescriptor(void) const
		{
			return *descriptor;
		}

		/* Updates the uniform block if needed. */
		void Update(_In_ CommandBuffer &cmdBuffer);

	protected:
		/* Specifies whether the uniforms need to be updated on the GPU. */
		bool IsDirty;

		/* Initializes a new instance of a uniform block. */
		UniformBlock(_In_ LogicalDevice &device, _In_ size_t size, _In_ const DescriptorPool &pool, _In_ uint32 set);

		/* Gets the object that describes the uniform buffer. */
		_Check_return_ inline DescriptorSet& GetDescriptor(void)
		{
			return *descriptor;
		}

		/* Loads the specified staging buffer with the new GPU data. */
		virtual void Stage(_In_ byte *destination) = 0;
		/* Updates the descriptor uniforms. */
		virtual void UpdateDescriptor(_In_ DescriptorSet &descriptor, _In_ const Buffer &uniformBuffer) = 0;

	private:
		DescriptorSet *descriptor;
		DynamicBuffer *target;
		bool firstUpdate;

		void Destroy(void);
	};
}