#pragma once
#include <queue>
#include "Graphics/Vulkan/CommandBuffer.h"

namespace Pu
{
	/* Defines a safe way to query occlusion and timestamp information from the GPU.s */
	class QueryChain
		: private QueryPool
	{
	public:
		/* Initializes a new instance of a query chain for either occlusion or timestamp queries. */
		QueryChain(_In_ LogicalDevice &device, _In_ QueryType type, _In_ size_t bufferCount = 2);
		QueryChain(_In_ const QueryChain&) = delete;
		/* Move constructor. */
		QueryChain(_In_ QueryChain &&value) = default;

		_Check_return_ QueryChain& operator =(_In_ const QueryChain&) = delete;
		/* Move assignment. */
		_Check_return_ QueryChain& operator =(_In_ QueryChain &&other) = default;

		/* Resets the queries that need to be reset. */
		void Reset(_In_ CommandBuffer &cmdBuffer);
		/* Records a timestamp to the pool at a specified pipeline stage. */
		void RecordTimestamp(_In_ CommandBuffer &cmdBuffer, _In_ PipelineStageFlag stage);
		/* Gets the time between the two timestamps previously recorded to the command buffer. */
		_Check_return_ float GetTimeDelta(void) const;

		/* Gets the time between two timestamps in milliseconds (used for the profiler). */
		_Check_return_ inline int64 GetProfilerTimeDelta(void) const
		{
			return static_cast<int64>(GetTimeDelta() * 0.001f);
		}

	private:
		mutable std::queue<uint32> chain;
		byte flags;

#ifdef _DEBUG
		bool logged;
#endif

		uint32 GetNext(void) const;
	};
}