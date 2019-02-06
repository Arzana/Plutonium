#pragma once

namespace Pu
{
	/* Defines how an asset can be copied. */
	enum class DuplicationType
	{
		/* The assets reference count will increase and a reference will be given. */
		Reference,
		/* The asset will be copied, no reference count will be increased. */
		MemberwiseCopy
	};
}