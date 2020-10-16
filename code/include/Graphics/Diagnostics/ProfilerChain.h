#pragma once
#include "QueryChain.h"

namespace Pu
{
	/* Defines the intermediate layer between GPU timestamp queries and the profiler. */
	class ProfilerChain
		: public QueryChain
	{
	public:
		/* Initializes a new instance of a query chain for GPU profiler delta times. */
		ProfilerChain(_In_ LogicalDevice &device, _In_ size_t chainCount, _In_opt_ size_t bufferCount = 2);
		/* Initializes a new instance of a single query chain for GPU profiler delta times. */
		ProfilerChain(_In_ LogicalDevice &device, _In_ const string &category, _In_ Color color, _In_opt_ size_t bufferCount = 2);
		ProfilerChain(_In_ const ProfilerChain&) = delete;
		/* Move constructor. */
		ProfilerChain(_In_ ProfilerChain &&value) = default;

		_Check_return_ ProfilerChain& operator =(_In_ const ProfilerChain&) = delete;
		/* Move assignment. */
		_Check_return_ ProfilerChain& operator =(_In_ ProfilerChain &&other) = default;

		/* Sets the profiler information for a specific chain in the query pool. */
		void SetChainInfo(_In_ uint32 chain, _In_ const string &category, _In_ Color color);

	private:
		friend class Profiler;

		vector<std::pair<string, Color>> chainInfo;
	};
}