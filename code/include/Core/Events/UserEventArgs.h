#pragma once
#include <sal.h>
#include "EventArgs.h"

namespace Pu
{
	/* Defines event arguments with a user defined parameter. */
	struct UserEventArgs
		: public EventArgs
	{
	public:
		/* The parameter send by the user. */
		const void *UserParam;

		/* Initializes an empty instance of user event arguments. */
		UserEventArgs(void)
			: UserParam(nullptr)
		{}

		/* Initializes a new instance of the user event argument object. */
		UserEventArgs(_In_ const void *param)
			: UserParam(param)
		{}
	};
}