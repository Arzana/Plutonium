#pragma once
#include "VulkanProcedres.h"

namespace Pu
{
	/* Defines a logical device command queue. */
	class Queue
	{
	public:
		Queue(_In_ const Queue&) = delete;
		/* Move constructor. */
		Queue(_In_ Queue &&value);

		_Check_return_ Queue& operator =(_In_ const Queue&) = delete;
		/* Move assignment. */
		_Check_return_ Queue& operator =(_In_ Queue &&other);

	private:
		friend class LogicalDevice;

		QueueHndl hndl;

		Queue(QueueHndl hndl);
	};
}