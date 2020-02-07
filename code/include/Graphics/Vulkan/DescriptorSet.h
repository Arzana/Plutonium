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
		DescriptorSet(_In_ const DescriptorPool &pool, _In_ const Pipeline &pipeline, _In_ uint32 set);
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
		/* Writes uniform block data to the set, represented by multiple descriptors. */
		void Write(_In_ const vector<const Descriptor*> &descriptors);
		/* Free's the descriptor set from its parent pool. */
		void Free(void);

	private:
		DescriptorSetHndl hndl;
		const DescriptorPool *pool;
		uint32 set;
		DeviceSize baseOffset;

		void ValidateDescriptor(const Descriptor &descriptor, DescriptorType type) const;
		void WriteDescriptors(const vector<WriteDescriptorSet> &writes);
		void Destroy(void);
	};
}