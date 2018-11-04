#pragma once
#include <sal.h>
#include "Core\Math\Constants.h"

namespace Plutonium
{
#if defined (_WIN32)
	/* Defines a structure that handles fetching values from the Windows registry. */
	class RegistryFetcher
	{
	public:
		/* Attempts to read a DWORD (int32) from the registry. */
		static bool TryReadInt32(_In_ const char *key, _In_ const char *subKey, _Out_ int32 *result);
		/* Attempts to read a QWORD (int64) from the registry. */
		static bool TryReadInt64(_In_ const char *key, _In_ const char *subKey, _Out_ int64 *result);
		/* Attempts to read a EXPAND_SZ (string) from the registry (result needs to be freed!). */
		static bool TryReadString(_In_ const char *key, _In_ const char *subKey, _Out_ const char **result);

	private:
		static bool TryReadRaw(const char *key, const char *subKey, void *data, size_t *dataSize);
	};
#endif
}