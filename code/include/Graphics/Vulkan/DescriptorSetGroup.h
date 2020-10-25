#pragma once
#include "DescriptorSetBase.h"

namespace Pu
{
	/* Defines multiple sets of descriptors. */
	class DescriptorSetGroup
		: public DescriptorSetBase
	{
	public:
		/* Initializes an empty instance of a descriptor set group. */
		DescriptorSetGroup(_In_ DescriptorPool &pool);
		DescriptorSetGroup(_In_ const DescriptorSetGroup&) = delete;
		/* Move constructor. */
		DescriptorSetGroup(_In_ DescriptorSetGroup &&value);
		/* Releases the resources allocated by the descriptor set group. */
		virtual ~DescriptorSetGroup(void)
		{
			Free();
		}

		_Check_return_ DescriptorSetGroup& operator =(_In_ const DescriptorSetGroup&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorSetGroup& operator =(_In_ DescriptorSetGroup &&other);
		
		/* Adds a descriptor set layout from a specific subpass to this grouping, returns the offset of this set in the staging buffer. */
		_Check_return_ DeviceSize Add(_In_ uint32 subpass, _In_ const DescriptorSetLayout &layout);
		/* Writes an input attachment to the set. */
		void Write(_In_ uint32 subpass, _In_ const Descriptor &descriptor, _In_ const TextureInput &input);
		/* Writes a depth buffer as an input attachment to the set. */
		void Write(_In_ uint32 subpass, _In_ const Descriptor &descriptor, _In_ const DepthBuffer &input);
		/* Writes an image/sampler combination to the specified set. */
		void Write(_In_ uint32 subpass, _In_ const Descriptor &descriptor, _In_ const Texture &texture);
		/* Writes a storage image to the specified set. */
		void Write(_In_ uint32 subpass, _In_ const Descriptor &descriptor, _In_ const ImageView &image);
		/* Writes a storage buffer to the specified set. */
		void Write(_In_ uint32 subpass, _In_ const Descriptor &descriptor, _In_ const Buffer &buffer);
		/* Frees the descriptor sets from their parent pool. */
		void Free(void);

	protected:
		/* Copies the block data to the CPU staging buffer. */
		virtual void Stage(_In_ DescriptorPool& /*sender*/, _In_ byte* /*destination*/) {};

	private:
		friend class CommandBuffer;

		std::map<uint64, DescriptorSetHndl> hndls;
		bool subscribe;

		DescriptorSetHndl GetSetHandle(uint32 subpass, const Descriptor &descriptor) const;
	};
}