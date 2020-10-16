#pragma once
#include <queue>
#include "Graphics/Vulkan/CommandBuffer.h"

namespace Pu
{
	/* Defines a safe way to query occlusion and timestamp information from the GPU. */
	class QueryChain
		: private QueryPool
	{
	public:
		/* Initializes a new instance of a query chain for either occlusion or timestamp queries. */
		QueryChain(_In_ LogicalDevice &device, _In_ QueryType type, _In_ size_t chainCount, _In_opt_ size_t bufferCount = 2);
		/* Initializes a new instance of a query chain for pipeline statisics queries. */
		QueryChain(_In_ LogicalDevice &device, _In_ QueryPipelineStatisticFlags statistics, _In_opt_ size_t bufferCount = 2);
		QueryChain(_In_ const QueryChain&) = delete;
		/* Move constructor. */
		QueryChain(_In_ QueryChain &&value) = default;

		_Check_return_ QueryChain& operator =(_In_ const QueryChain&) = delete;
		/* Move assignment. */
		_Check_return_ QueryChain& operator =(_In_ QueryChain &&other) = default;

		/* Resets the queries that need to be reset. */
		void Reset(_In_ CommandBuffer &cmdBuffer);
		/* Records a timestamp to the pool at a specified pipeline stage. */
		void RecordTimestamp(_In_ CommandBuffer &cmdBuffer, _In_ uint32 chain, _In_ PipelineStageFlags stage);
		/* Starts or ends the recording of a pipeline statistics query. */
		void RecordStatistics(_In_ CommandBuffer &cmdBuffer);
		/* Gets the time between the two timestamps previously recorded to the command buffer. */
		_Check_return_ float GetTimeDelta(_In_ uint32 chain) const;
		/* Gets the results of the pipeline queries. */
		_Check_return_ vector<uint32> GetStatistics(void) const;
		/* Gets the human readable version of the pipeline statistic at the specified index. */
		_Check_return_ const char* GetStatisticName(_In_ uint32 idx) const;

	private:
		struct Chain
		{
			std::queue<uint32> chain;
			uint32 first, last;
			byte flags;

			Chain(QueryType type, size_t bufferCount, size_t idx);

			uint32 GetNext(void) const;
		};

		mutable vector<Chain> chains;
		mutable QueryPipelineStatisticFlags stats;

#ifdef _DEBUG
		bool logged;
#endif
	};
}