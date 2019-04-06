#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Defines an object used to load run-time libraries. */
	class DynamicLibLoader
	{
	public:
		/* Initializes a new instance of a dynamic lib loader. */
		DynamicLibLoader(_In_ const wstring &name);
		DynamicLibLoader(_In_ const DynamicLibLoader&) = delete;
		/* Move constructor. */
		DynamicLibLoader(_In_ DynamicLibLoader &&value);
		/* Releases the library. */
		virtual ~DynamicLibLoader()
		{
			Destroy();
		}

		_Check_return_ DynamicLibLoader& operator =(_In_ const DynamicLibLoader&) = delete;
		/* Move assignment. */
		_Check_return_ DynamicLibLoader& operator =(_In_ DynamicLibLoader &&other);

		/* Gets whether the loader successfully loaded the dynamic library. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return hlib;
		}

		/* Loads a specified process for the library. */
		template <typename proc_t>
		_Check_return_ proc_t LoadProc(_In_ const char *name) const
		{
			return reinterpret_cast<proc_t>(LoadRawProc(name));
		}

	private:
		void* hlib;

		void* LoadRawProc(const char *name) const;
		void Destroy();
	};
}