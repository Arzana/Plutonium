#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a manager for GPU queries. */
	class QueryPool
	{
	public:
		/* Initializes a new instance of a query pool for either occlusion or timestamp queries. */
		QueryPool(_In_ LogicalDevice &device, _In_ QueryType type, _In_ size_t count);
		/* Initializes a new instance of a query pool for pipeline statistics queries. */
		QueryPool(_In_ LogicalDevice &device, _In_ size_t count, _In_ QueryPipelineStatisticFlag statistics);
		QueryPool(_In_ const QueryPool&) = delete;
		/* Move constructor. */
		QueryPool(_In_ QueryPool &&value);
		/* Destroys the query pool. */
		~QueryPool(void)
		{
			Destroy();
		}

		_Check_return_ QueryPool& operator =(_In_ const QueryPool&) = delete;
		/* Move assignment. */
		_Check_return_ QueryPool& operator =(_In_ QueryPool &&other);

		/* Attempts to get the result from specific queries. */
		vector<uint32> GetResults(_In_ uint32 firstQuery, _In_ uint32 queryCount, _In_ bool wait, _In_ bool partial) const;
		/* Gets the difference between two timesteps (in nanoseconds) from specific queries. */
		float GetTimeDelta(_In_ uint32 firstQuery, bool wait);

	private:
		friend class CommandBuffer;

		LogicalDevice *parent;
		QueryPoolHndl hndl;

		void Destroy(void);
	};
}