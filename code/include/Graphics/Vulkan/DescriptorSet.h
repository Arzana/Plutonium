#pragma once
#include "DescriptorPool.h"
#include "Pipelines/Pipeline.h"
#include "Graphics/Textures/TextureInput.h"

namespace Pu
{
	/* Defines a set of descriptors. */
	class DescriptorSet
	{
	public:
		/* Initializes a new instance of a descriptor set from a specific pool. */
		DescriptorSet(_In_ DescriptorPool &pool, _In_ uint32 set);
		DescriptorSet(_In_ const DescriptorSet&) = delete;
		/* Move constructor. */
		DescriptorSet(_In_ DescriptorSet &&value);
		/* Releases the resources allocated by the descriptor set. */
		~DescriptorSet(void)
		{
			Destroy();
		}

		_Check_return_ DescriptorSet& operator =(_In_ const DescriptorSet&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorSet& operator =(_In_ DescriptorSet &&other);

		/* Writes an input attachment to the set. */
		void Write(_In_ const Descriptor &descriptor, _In_ const TextureInput &input);
		/* Writes an image/sampler combination to the set. */
		void Write(_In_ const Descriptor &descriptor, _In_ const Texture &texture);
		/* Free's the descriptor set from its parent pool. */
		void Free(void);

	protected:
		/* Copies the block data to the CPU staging buffer. */
		virtual void Stage(_In_ byte* /*destination*/) {};

		/* Copies the specific value into the array. */
		template <typename value_t>
		static inline void Copy(_In_ byte *destination, _In_ const value_t *value)
		{
			memcpy(destination, value, sizeof(std::remove_pointer_t<value_t>));
		}

		/* Gets the specific descriptor from the specific subpass. */
		_Check_return_ inline const Descriptor& GetDescriptor(_In_ uint32 subpass, _In_ const string &name) const
		{
			return pool->renderpass->GetSubpass(subpass).GetDescriptor(name);
		}

	private:
		friend class CommandBuffer;

		DescriptorSetHndl hndl;
		DescriptorPool *pool;
		uint32 set;
		DeviceSize baseOffset;

		void WriteBuffer(void);
		void SubscribeIfNeeded(void);
		void StageInternal(DescriptorPool&, byte *destination);
		void ValidateDescriptor(const Descriptor &descriptor, DescriptorType type) const;
		void WriteDescriptors(const vector<WriteDescriptorSet> &writes);
		void Destroy(void);
	};
}