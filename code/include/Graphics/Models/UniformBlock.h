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

		/* Gets the GPU size (in bytes) for the target buffer. */
		_Check_return_ inline size_t GetSize(void) const
		{
			return target->GetSize();
		}

		/* Forces the material to be updated on the GPU on the next Update call. */
		inline void ForceUpdate(void)
		{
			IsDirty = true;
		}

	protected:
		/* Specifies whether the uniforms need to be updated on the GPU. */
		bool IsDirty;

		/* Initializes a new instance of a uniform block. */
		UniformBlock(_In_ DescriptorPool &pool);

		/* Loads the specified staging buffer with the new GPU data. */
		virtual void Stage(_In_ byte *destination) = 0;

		/* Performs a memcpy to the destination from a single value pointer. */
		template <typename value_t>
		static inline void Copy(_In_ byte *dest, _In_ const value_t *value)
		{
			memcpy(dest, value, sizeof(std::remove_pointer_t<value_t>));
		}

	private:
		DynamicBuffer *target;
		vector<const Descriptor*> descriptors;
		bool firstUpdate;

		void Destroy(void);
	};
}