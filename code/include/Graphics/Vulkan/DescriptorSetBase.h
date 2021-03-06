#pragma once
#include "DescriptorPool.h"
#include "Graphics/Textures/TextureInput.h"
#include "Graphics/Textures/TextureStorage.h"
#include "Graphics/Textures/DepthBuffer.h"

namespace Pu
{
	/* Defines a base class for the descriptor set and descriptor set group. */
	class DescriptorSetBase
	{
	public:
		/* Copy constructor. */
		DescriptorSetBase(_In_ const DescriptorSetBase &value) = default;
		/* Move constructor. */
		DescriptorSetBase(_In_ DescriptorSetBase &&value) = default;
		/* Releases the resources allocated by the descriptor set base. */
		virtual ~DescriptorSetBase(void) { }

		/* Copy assignment. */
		_Check_return_ DescriptorSetBase& operator =(_In_ const DescriptorSetBase &other) = default;
		/* Move assignment. */
		_Check_return_ DescriptorSetBase& operator =(_In_ DescriptorSetBase &&other) = default;

	protected:
		/* Defines the parent descriptor pool. */
		DescriptorPool *Pool;

		/* Initializes a new instance of a descriptor set base with a specified parent pool. */
		DescriptorSetBase(_In_ DescriptorPool &pool);

		/* Writes the specific descriptor set layout to the parent pool buffer. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ const DescriptorSetLayout &layout, _In_ DeviceSize offset);
		/* Writes an input attachment to the specified set. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const TextureInput &input);
		/* Writes a deth buffer attachment to the specified set as an input attachment. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const DepthBuffer &input);
		/* Writes a storage image to the set. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const TextureStorage &image);
		/* Writes an image/sampler combination to the set. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const Texture &texture);
		/* Writes a storage buffer to the set. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const Buffer &buffer);
		/* Gets the aligned offset for descriptors in a set that is present in multiple shader stages. */
		_Check_return_ DeviceSize GetOffsetAligned(_In_ DeviceSize size) const;

		/* Copies the specific value into the array. */
		template <typename value_t>
		static inline void Copy(_In_ byte *destination, _In_ const value_t *value)
		{
			memcpy(destination, value, sizeof(value_t));
		}

		/* Gets the specific descriptor from the specific subpass (Only valid for renderpass descriptors). */
		_Check_return_ inline const Descriptor& GetDescriptor(_In_ uint32 subpass, _In_ const string &name) const
		{
			return Pool->renderpass->GetSubpass(subpass).GetDescriptor(name);
		}

		/* Gets the specific descriptor (Only valid for compute pass descriptors). */
		_Check_return_ inline const Descriptor& GetDescriptor(_In_ const string &name) const
		{
			return Pool->computepass->GetDescriptor(name);
		}

	private:
		static void ValidateDescriptor(const Descriptor &descriptor, uint32 set, DescriptorType type);

		void WriteNonSampled(DescriptorSetHndl setHndl, uint32 set, const Descriptor &descriptor, ImageViewHndl viewHndl, DescriptorType type, ImageLayout layout);
		void WriteDescriptors(const vector<WriteDescriptorSet> &writes);
	};
}