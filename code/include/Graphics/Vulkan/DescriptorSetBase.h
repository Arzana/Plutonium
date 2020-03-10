#pragma once
#include "DescriptorPool.h"
#include "Graphics/Textures/TextureInput.h"
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
		/* Writes an image/sampler combination to the set. */
		void Write(_In_ DescriptorSetHndl hndl, _In_ uint32 set, _In_ const Descriptor &descriptor, _In_ const Texture &texture);

		/* Copies the specific value into the array. */
		template <typename value_t>
		static inline void Copy(_In_ byte *destination, _In_ const value_t *value)
		{
			memcpy(destination, value, sizeof(value_t));
		}

		/* Gets the specific descriptor from the specific subpass. */
		_Check_return_ inline const Descriptor& GetDescriptor(_In_ uint32 subpass, _In_ const string &name) const
		{
			return Pool->renderpass->GetSubpass(subpass).GetDescriptor(name);
		}

	private:
		static void ValidateDescriptor(const Descriptor &descriptor, uint32 set, DescriptorType type);

		void WriteInput(DescriptorSetHndl setHndl, uint32 set, const Descriptor &descriptor, _In_ ImageViewHndl viewHndl);
		void WriteDescriptors(const vector<WriteDescriptorSet> &writes);
	};
}