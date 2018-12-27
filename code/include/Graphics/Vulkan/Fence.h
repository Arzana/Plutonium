#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a synchronization object that can be used to communicate that some task has completed. */
	class Fence
	{
	public:
		/* Initializes a new instanace of a fence. */
		Fence(_In_ LogicalDevice &device, _In_opt_ bool signaled = false);
		Fence(_In_ const Fence&) = delete;
		/* Move constructor. */
		Fence(_In_ Fence &&value);
		/* Destroys the fence. */
		~Fence(void)
		{
			Destroy();
		}

		_Check_return_ Fence& operator =(_In_ const Fence&) = delete;
		/* Move assignment. */
		_Check_return_ Fence& operator =(_In_ Fence &&other);

		/* Queries whether the fence status and returns if it's a success result. */
		_Check_return_ bool IsSignaled(void) const;
		/* Resets the fence to an unsignaled state. */
		void Reset(void);
		/* Halts the current thread until the fence is signaled or until the timeout (in nanoseconds) has passed, returns the new state of the fence. */
		_Check_return_ bool Wait(_In_opt_ uint64 timeout = maxv<uint64>()) const;
		/* Halts the current thread until the specified fences are signaled or until the timeout (in nanoseconds) has passed. */
		_Check_return_ static bool WaitAll(_In_ const LogicalDevice &device, _In_ const vector<const Fence*> &fences, _In_opt_ uint64 timeout = maxv<uint64>());
		/* Halts the current thread until one of the specified fences has been signaled or until the timeout (in nanoseconds) has passed. */
		_Check_return_ static bool WaitAny(_In_ const LogicalDevice &device, _In_ const vector<const Fence*> &fences, _In_opt_ uint64 timeout = maxv<uint64>());

	private:
		friend class Queue;

		FenceHndl hndl;
		LogicalDevice &parent;

		static bool WaitInternal(const LogicalDevice &device, uint32 fenceCnt, const FenceHndl *fences, bool waitAll, uint64 timeout);
		void Destroy(void);
	};
}