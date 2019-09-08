#pragma once

namespace Pu
{
	/* Defines the types of methods for a HTTP(U) request. */
	enum class HttpMethod
	{
		/* Request a specific resource (default). */
		Get,
		/* Request a response identical to a get request but without the body. */
		Head,
		/* Used to submit an entry to a specific resource. */
		Post,
		/* Used to replace a specific entry to a specific resource. */
		Put,
		/* Used to delete a specific entry in the specified resource. */
		Delete,
		/* Establishes a connection to the sever. */
		Connect,
		/* Used to describe the communication options for the target resource. */
		Options,
		/* Performs a message loop-back test. */
		Trace,
		/* Used to partially modify a resource. */
		Patch
	};
}