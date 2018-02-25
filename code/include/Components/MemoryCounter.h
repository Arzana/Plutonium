#pragma once
#include "Core\Diagnostics\Memory.h"
#include "GameComponent.h"
#include <deque>

namespace Plutonium
{
	/* Provides a simple memory counter with a specified buffer size. */
	struct MemoryCounter 
		: public GameComponent
	{
	public:
		/* Initializes a new instance of an memory counter. */
		MemoryCounter(_In_ Game *game, _In_ size_t bufferSize = 100);
		MemoryCounter(_In_ const MemoryCounter &value) = delete;
		MemoryCounter(_In_ MemoryCounter &&value) = delete;

		_Check_return_ MemoryCounter& operator =(_In_ const MemoryCounter &other) = delete;
		_Check_return_ MemoryCounter& operator =(_In_ MemoryCounter &&other) = delete;

		/* Gets the current virtual memory usage. */
		_Check_return_ uint64 GetCurVRamUsage(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot get current used bytes when no frame has been updated yet!");
			return buffer.at(buffer.size() - 1).UsedVRam;
		}

		/* Gets the current physical memory usage. */
		_Check_return_ uint64 GetCurRamUsage(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot get current used bytes when no frame has been updated yet!");
			return buffer.at(buffer.size() - 1).UsedRam;
		}

		/* Gets the worst virtual memory usage. */
		_Check_return_ uint64 GetWorstVRamUsage(void) const
		{
			return vworst;
		}

		/* Gets the worst physical memory usage. */
		_Check_return_ uint64 GetWorstRamUsage(void) const
		{
			return worst;
		}

		/* Gets the best virtual memory usage. */
		_Check_return_ uint64 GetBestVRamUsage(void) const
		{
			return vbest;
		}

		/* Gets the best physical memory usage. */
		_Check_return_ uint64 GetBestRamUsage(void) const
		{
			return best;
		}

		/* Gets the average virtual memory usage. */
		_Check_return_ uint64 GetAvrgVRamUsage(void) const
		{
			return vavrg;
		}

		/* Gets the average physical memory usage. */
		_Check_return_ uint64 GetAvrgRamUsage(void) const
		{
			return avrg;
		}

		/* Gets the total virtual memory from the OS. */
		_Check_return_ uint64 GetOSVRamBudget(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot get total bytes when no frame has been updated yet!");
			return buffer.at(buffer.size() - 1).TotalVRam;
		}

		/* Gets the total physical memory from the OS. */
		_Check_return_ uint64 GetOSRamBudget(void) const
		{
			ASSERT_IF(buffer.size() < 1, "Buffer empty!", "Cannot get total bytes when no frame has been updated yet!");
			return buffer.at(buffer.size() - 1).TotalRam;
		}

	protected:
		/* Updates the buffer of the memory counter. */
		virtual void Update(_In_ float dt) override;

	private:
		size_t maxSize;
		std::deque<MemoryFrame> buffer;
		uint64 avrg, worst, best;
		uint64 vavrg, vworst, vbest;
	};
}