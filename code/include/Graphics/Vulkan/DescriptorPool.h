#pragma once
#include "Shaders/Renderpass.h"

namespace Pu
{
	class DynamicBuffer;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Occurs when the descriptor pool is staging its memory. */
		EventBus<DescriptorPool, byte*> OnStage;

		/* Initializes a new instance of a descriptor pool. */
		DescriptorPool(_In_ const Renderpass &renderpass);
		/* Creates a descriptor pool for the specific descriptor set in the specific subpasses. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ uint32 maxSets, _In_ uint32 subpass, _In_ uint32 set);
		DescriptorPool(_In_ const DescriptorPool &value) = delete;
		/* Move constructor. */
		DescriptorPool(_In_ DescriptorPool &&value);
		/* Releases the resources allocated by the descriptor pool. */
		~DescriptorPool(void)
		{
			Destroy();
		}

		_Check_return_ DescriptorPool& operator =(_In_ const DescriptorPool&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorPool& operator =(_In_  DescriptorPool &&other);

		/* Adds a descriptor set from a specific subpass to the pool. */
		void AddSet(_In_ uint32 subpass, _In_ uint32 set, _In_ uint32 max);
		/* Updates the descriptor pool, staging all the descriptors to the GPU. */
		void Update(_In_ CommandBuffer &cmdBuffer, _In_ PipelineStageFlags dstStage);
		/* Recycles all the resources from all descriptor sets allocated from the pool back into the pool. */
		void Reset(void);

	private:
		friend class DescriptorSetBase;
		friend class DescriptorSet;
		friend class DescriptorSetGroup;
		friend class UniformBlock;

		struct SetInfo
		{
			uint64 Id;
			DeviceSize Offset;
			vector<DescriptorSetHndl> spaces;

			SetInfo(uint32 subpass, uint32 set, uint32 max, DeviceSize offset);

			inline uint32 GetMaxSets(void) const
			{
				return static_cast<uint32>(spaces.size());
			}
		};

		DescriptorPoolHndl hndl;
		DynamicBuffer *buffer;
		LogicalDevice *device;
		const Renderpass *renderpass;
		std::mutex lock;

		uint32 maxSets;
		DeviceSize stride;
		bool firstUpdate;
		vector<DescriptorPoolSize> sizes;
		vector<SetInfo> sets;

		DeviceSize Alloc(uint32 subpass, const DescriptorSetLayout &layout, DescriptorSetHndl *result);
		void Free(DescriptorSetHndl set);
		void Create(void);
		void Destroy(void);

		static void ThrowInvalidAlloc(uint32 subpass, uint32 set, const char *reason);
		static uint64 MakeId(uint32 subpass, uint32 set);
	};
}