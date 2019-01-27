#pragma once
#include "Graphics/Vulkan/Shaders/Uniform.h"
#include "Graphics/Textures/Texture.h"
#include "Graphics/Vulkan/Buffer.h"

namespace Pu
{
	class DescriptorPool;

	/* Defines a set of descriptors. */
	class DescriptorSet
	{
	public:
		DescriptorSet(_In_ const DescriptorSet&) = delete;
		/* Move constructor. */
		DescriptorSet(_In_ DescriptorSet &&value);
		/* Frees the descriptor set. */
		~DescriptorSet(void)
		{
			Free();
		}

		_Check_return_ DescriptorSet& operator =(_In_ const DescriptorSet&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorSet& operator =(_In_ DescriptorSet &&other);

		/* Writes image data to a specific binding (represented by the uniform). */
		void Write(_In_ const Uniform &uniform, _In_ const Texture &texture);
		/* Writes buffer data to a specific binding (represented by the uniform). */
		void Write(_In_ const Uniform &uniform, _In_ const Buffer &buffer);
		/* Writes image data to a specific binding (represented by the uniform). */
		void Write(_In_ const Uniform &uniform, _In_ const vector<const Texture*> &textures);

	private:
		friend class DescriptorPool;
		friend class CommandBuffer;

		DescriptorPool &parent;
		DescriptorSetHndl hndl;

		DescriptorSet(DescriptorPool &pool, DescriptorSetHndl hndl);

		void WriteImage(const Uniform &uniform, const vector<DescriptorImageInfo> &infos);
		void WriteBuffer(const Uniform &uniform, const vector<DescriptorBufferInfo> &infos);
		void WriteDescriptor(const vector<WriteDescriptorSet> &writes);
		void UpdateDescriptor(size_t writeCount, const WriteDescriptorSet *writes, size_t copyCount, const CopyDescriptorSet *copies);
		void Free(void);
	};
}