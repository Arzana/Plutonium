#pragma once
#include "DescriptorSetBase.h"

namespace Pu
{
	/* Defines a set of descriptors. */
	class DescriptorSet
		: public DescriptorSetBase
	{
	public:
		/* Initializes a new instance of a descriptor set from a specific pool. */
		DescriptorSet(_In_ DescriptorPool &pool, _In_ uint32 subpass, _In_ const DescriptorSetLayout &setLayout);
		DescriptorSet(_In_ const DescriptorSet&) = delete;
		/* Move constructor. */
		DescriptorSet(_In_ DescriptorSet &&value);
		/* Releases the resources allocated by the descriptor set. */
		virtual ~DescriptorSet(void)
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

	private:
		friend class CommandBuffer;

		DescriptorSetHndl hndl;
		uint32 set;
		DeviceSize baseOffset;
		bool subscribe;

		void StageInternal(DescriptorPool&, byte *destination);
		void Destroy(void);
	};
}