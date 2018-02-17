#pragma once
#include <sal.h>

namespace Plutonium
{
	/* Defines a helper object for loading content. */
	struct ContentLoader
	{
	public:
		/* Initializes a new instance of a content loader with a specified root directory. */
		ContentLoader(void);
		ContentLoader(_In_ const ContentLoader &value) = delete;
		ContentLoader(_In_ ContentLoader &&value) = delete;

		_Check_return_ ContentLoader& operator =(_In_ const ContentLoader &other) = delete;
		_Check_return_ ContentLoader& operator =(_In_ ContentLoader &&other) = delete;

		/* Gets the current directory from where all the paths originate. */
		_Check_return_ inline const char* GetRoot(void) const
		{
			return root;
		}

		/* Changes the current root directory to the specified path. */
		void SetRoot(_In_ const char *directory);

	private:
		const char * root;
		size_t rootLen;
	};
}