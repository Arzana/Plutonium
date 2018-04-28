#pragma once
#include <cstdlib>
#include <malloc.h>
#include <type_traits>
#include "Core\Diagnostics\Logging.h"

/* Converts the specified pointer to a void pointer. */
#define void_ptr(ptr)					reinterpret_cast<const void*>(ptr)
/* Gets the byte offset to the specified member in the specified type as a void pointer. */
#define offset_ptr(o, m)				void_ptr(offsetof(o, m))

#if defined(DEBUG)
#define malloc_s(type, size)			Plutonium::_CrtMallocH<type>(size)
#define malloca_s(type, size)			Plutonium::_CrtMallocS<type>(size)
#define realloc_s(type, block, size)	Plutonium::_CrtReallocH<type>(&block, size)
#define calloc_s(type, size)			Plutonium::_CrtCallocH<type>(size)

#define free_s(block)					Plutonium::_CrtFreeH(&block)
#define freea_s(block)					Plutonium::_CrtFreeS(&block)
#define delete_s(block)					Plutonium::_CrtDelete(&block)
#else
#define malloc_s(type, size)			reinterpret_cast<type*>(malloc((size) * sizeof(type)))
#define malloca_s(type, size)			reinterpret_cast<type*>(_malloca((size) * sizeof(type)))
#define realloc_s(type, block, size)	(block = reinterpret_cast<type*>(realloc(block, (size) * sizeof(type))))
#define calloc_s(type, size)			reinterpret_cast<type*>(calloc((size), sizeof(type)))

#define free_s(block)					free(const_cast<void*>(void_ptr(block)))
#define freea_s(block)					_freea(const_cast<void*>(void_ptr(block)))
#define delete_s(block)					delete block
#endif	

namespace Plutonium
{
	/* Allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtMallocH(_In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		LOG_THROW_IF(size < 1, "cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = malloc(size);
		LOG_THROW_IF(result == nullptr, "cannot allocate %zu objects (insufficient memory)!", amount);

		/* Return type safe pointer. */
		return reinterpret_cast<_Ty*>(result);
	}

	/* Allocates C-style memory on the stack. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtMallocS(_In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		LOG_THROW_IF(size < 1, "cannot allocate %zu objects (byte size %zu)!", amount, size);

		void *result = nullptr;
		__try
		{
			/* Try allocate memory. */
			result = _malloca(size);
		}
		__except (1)
		{
			/* _malloca doesn't return nullptr on stackoverflow but generates a SEH exception. */
			LOG_WAR_IF(_resetstkoflw(), "Could not reset stack!");
			LOG_THROW("Cannot allocate %zu objects (insufficient memory)!", amount);
		}

		/* Return type safe pointer. */
		return reinterpret_cast<_Ty*>(result);
	}

	/* Re-allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline void _CrtReallocH(_In_ _Ty **block, _In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		LOG_THROW_IF(size < 1, "cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = realloc(*block, size);
		LOG_THROW_IF(result == nullptr, "cannot allocate %zu objects (insufficient memory)!", amount);

		/* Set old block to new value. */
		*block = reinterpret_cast<_Ty*>(result);
	}

	/* Allocates a C-style memory on the heap and zero's out the bytes. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtCallocH(_In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		LOG_THROW_IF(size < 1, "cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = calloc(amount, sizeof(_Ty));
		LOG_THROW_IF(result == nullptr, "cannot allocate %zu objects (insufficient memory)!", amount);

		/* Return type safe pointer. */
		return reinterpret_cast<_Ty*>(result);
	}

	/* Deletes a C-style memory block and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtFreeH(_In_ _Ty **block)
	{
		/* Check for nullptr. */
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");

		/* Release and set to nullptr. */
		free(const_cast<void*>(void_ptr((*block))));
		*block = nullptr;
	}

	template <typename _Ty>
	inline void _CrtFreeS(_In_ _Ty **block)
	{
		/* Check for nullptr. */
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");

		/* Release and set to nullptr. */
		_freea(const_cast<void*>(void_ptr(*block)));
		*block = nullptr;
	}

	/* Deletes a pointer and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtDelete(_In_ _Ty **block)
	{
		/* Check for nullptr. */
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");

		/* Release and set to nullptr. */
		delete *block;
		*block = nullptr;
	}
}