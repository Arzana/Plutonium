#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Defines the components of a SSDP response. */
	struct SSDPResponse
	{
		/* The HTTP status code of the response. */
		uint16 StatusCode;
		/* The Time to Live of the message. */
		uint32 TTL;
		/* 
		The UUID of the device.
		[TTTTTTTT-TTTT-VTTT-SSSS-AAAAAAAAAAAA]
		T = generated
		V = UUID version
		S = clock sequence
		A = MAC address
		*/
		string UUID;
		/* The (company) name of the server on which the service resides. */
		string ServerName;
		/* The Operating System of the server. */
		string OS;
		/* The version of the requested service. */
		string ServiceVersion;
		/* The product associated with the requested service. */
		string Product;
		/* The version of the product. */
		string ProductVersion;
		/* The network location (URL) of the device description. */
		string Location;
	};
}