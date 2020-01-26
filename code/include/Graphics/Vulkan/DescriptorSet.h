#pragma once
#include "Graphics/Vulkan/Shaders/Descriptor.h"
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

		/* Writes an image with a specific sampler to the set (represented by a descriptor). */
		void Write(_In_ const Descriptor &descriptor, _In_ const ImageView &view, _In_ const Sampler &sampler);
		/* Writes an input attachment image to the set (represented by a descriptor). */
		void Write(_In_ const Descriptor &descriptor, _In_ const ImageView &view);
		/* Writes buffer data to the set (represented by the descriptor). */
		void Write(_In_ const vector<const Descriptor*> &descriptors, _In_ const Buffer &buffer);
		/* Writes an image/sampler combination to the set (represented by the descriptor). */
		inline void Write(_In_ const Descriptor &descriptor, _In_ const Texture &texture)
		{
			Write(descriptor, *texture.view, *texture.Sampler);
		}

	private:
		friend class DescriptorPool;
		friend class CommandBuffer;

		DescriptorPool *parent;
		DescriptorSetHndl hndl;
		uint32 set;

		DescriptorSet(DescriptorPool &pool, DescriptorSetHndl hndl, uint32 set);

		void WriteDescriptor(const vector<WriteDescriptorSet> &writes);
		void Free(void);
	};
}