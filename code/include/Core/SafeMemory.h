#pragma once
#include <cstdlib>
#include <crtdbg.h>
#include <malloc.h>
#include <type_traits>
#include "Core/Diagnostics/Logging.h"

/* Converts the specified pointer to a void pointer. */
#define void_ptr(ptr)						reinterpret_cast<const void*>(ptr)
/* Gets the byte offset to the specified member in the specified type as a void pointer. */
#define offset_ptr(o, m)					void_ptr(offsetof(o, m))

#ifdef _DEBUG
#define malloc_s(type, size)				Pu::_CrtMallocH<type>(size)
#define realloc_s(type, block, size)		Pu::_CrtReallocH<type>(&block, size)
#define calloc_s(type, size)				Pu::_CrtCallocH<type>(size)

#define free_s(block)						Pu::_CrtFreeH(&block)
#define delete_s(block)						Pu::_CrtDelete(&block)
#else
#define malloc_s(type, size)				reinterpret_cast<type*>(malloc((size) * sizeof(type)))
#define malloca_s(type, size)				reinterpret_cast<type*>(salloc((size) * sizeof(type)))
#define realloc_s(type, block, size)		(block = reinterpret_cast<type*>(realloc(block, (size) * sizeof(type))))
#define calloc_s(type, size)				reinterpret_cast<type*>(calloc((size), sizeof(type)))

#define free_s(block)						free(const_cast<void*>(void_ptr(block)))
#define freea_s(block)						sfree(const_cast<void*>(void_ptr(block)))
#define delete_s(block)						delete block
#endif	

namespace Pu
{
	/* Allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtMallocH(_In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		if (size < 1) Log::Fatal("cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = malloc(size);
		if (!result) Log::Fatal("cannot allocate %zu objects (insufficient memory)!", amount);

		/* Return type safe pointer. */
		return reinterpret_cast<_Ty*>(result);
	}

	/* Re-allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline void _CrtReallocH(_In_ _Ty **block, _In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		if (size < 1) Log::Fatal("cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = realloc(*block, size);
		if (!result) Log::Fatal("cannot allocate %zu objects (insufficient memory)!", amount);

		/* Set old block to new value. */
		*block = reinterpret_cast<_Ty*>(result);
	}

	/* Allocates a C-style memory on the heap and zero's out the bytes. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtCallocH(_In_ size_t amount)
	{
		/* Check input size. */
		const size_t size = amount * sizeof(_Ty);
		if (size < 1) Log::Fatal("cannot allocate %zu objects (byte size %zu)!", amount, size);

		/* Allocate memory and check for insufficient memory. */
		void *result = calloc(amount, sizeof(_Ty));
		if (!result) Log::Fatal("cannot allocate %zu objects (insufficient memory)!", amount);

		/* Return type safe pointer. */
		return reinterpret_cast<_Ty*>(result);
	}

	/* Deletes a C-style memory block and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtFreeH(_In_ _Ty **block)
	{
		/* Check for nullptr. */
		if (!*block) Log::Error("Attempting to free nullptr!");

		/* Release and set to nullptr. */
		free(const_cast<void*>(void_ptr((*block))));
		*block = nullptr;
	}

	/* Deletes a pointer and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtDelete(_In_ _Ty **block)
	{
		/* Check for nullptr. */
		if(!*block) Log::Error("Attempting to free nullptr!");

		/* Release and set to nullptr. */
		delete *block;
		*block = nullptr;
	}
}