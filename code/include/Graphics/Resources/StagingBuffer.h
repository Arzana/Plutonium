#pragma once
#include "Graphics/Vulkan/Buffer.h"

namespace Pu
{
	/* Defines a buffer specialized for staging resources. */
	class StagingBuffer
		: public Buffer
	{
	public:
		/* Initializes a new instance of a staging buffer with a specified buffer target. */
		StagingBuffer(_In_ Buffer &target);
		/*Initialize a new instance of a staging buffer as a standalone buffer. */
		StagingBuffer(_In_ LogicalDevice &device, _In_ size_t size);
		StagingBuffer(_In_ const StagingBuffer&) = delete;
		/* Move assignment. */
		StagingBuffer(_In_ StagingBuffer &&value);

		_Check_return_ StagingBuffer& operator =(_In_ const StagingBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ StagingBuffer& operator =(_In_ StagingBuffer &&other);

		/* Sets the data of the staging buffer (expects the full data)! */
		void Load(const void *data);
	};
}