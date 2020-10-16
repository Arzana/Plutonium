#include "Graphics/Diagnostics/ProfilerChain.h"

Pu::ProfilerChain::ProfilerChain(LogicalDevice & device, size_t chainCount, size_t bufferCount)
	: QueryChain(device, QueryType::Timestamp, chainCount, bufferCount)
{
	chainInfo.resize(chainCount);
}

Pu::ProfilerChain::ProfilerChain(LogicalDevice & device, const string & category, Color color, size_t bufferCount)
	: QueryChain(device, QueryType::Timestamp, 1, bufferCount)
{
	chainInfo.emplace_back(std::make_pair(category, color));
}

void Pu::ProfilerChain::SetChainInfo(uint32 chain, const string & category, Color color)
{
	chainInfo.at(chain) = std::make_pair(category, color);
}