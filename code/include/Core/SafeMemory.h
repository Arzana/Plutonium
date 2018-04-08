#pragma once
#include <cstdlib>
#include <malloc.h>
#include <type_traits>
#include "Core\Diagnostics\Logging.h"

/* Converts the specified pointer to a void pointer. */
#define void_ptr(ptr)					reinterpret_cast<const void*>(ptr)
/* Gets the byte offset to the specified member in the specified type as a void pointer. */
#define offset_ptr(o, m)				void_ptr(offsetof(o, m))

#define malloc_s(type, size)			Plutonium::_CrtMallocH<type>(size)
#define malloca_s(type, size)			Plutonium::_CrtMallocS<type>(size)
#define realloc_s(type, block, size)	Plutonium::_CrtReallocH<type>(&block, size)
#define calloc_s(type, size)			Plutonium::_CrtCallocH<type>(size)

#if defined(DEBUG)
#define free_s(block)					Plutonium::_CrtFreeH(&block)
#define freea_s(block)					Plutonium::_CrtFreeS(&block)
#define delete_s(block)					Plutonium::_CrtDelete(&block)
#else
#define free_s(block)					free(const_cast<void*>(void_ptr(block))
#define freea_s(block)					_freea(block)
#define delete_s(block)					delete block
#endif	

namespace Plutonium
{
	/* Allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtMallocH(_In_ size_t amount)
	{
		return reinterpret_cast<_Ty*>(malloc(amount * sizeof(_Ty)));
	}

	/* Allocates C-style memory on the stack. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtMallocS(_In_ size_t amount)
	{
		return reinterpret_cast<_Ty*>(_malloca(amount * sizeof(_Ty)));
	}

	/* Re-allocates C-style memory on the heap. */
	template <typename _Ty>
	_Check_return_ inline void _CrtReallocH(_In_ _Ty **block, _In_ size_t amount)
	{
		*block = reinterpret_cast<_Ty*>(realloc(*block, amount * sizeof(_Ty)));
	}

	/* Allocates a C-style memory on the heap and zero's out the bytes. */
	template <typename _Ty>
	_Check_return_ inline _Ty* _CrtCallocH(_In_ size_t amount)
	{
		return reinterpret_cast<_Ty*>(calloc(amount * sizeof(_Ty)));
	}

	/* Deletes a C-style memory block and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtFreeH(_In_ _Ty **block)
	{
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");
		free(const_cast<void*>(void_ptr((*block))));
		*block = nullptr;
	}

	template <typename _Ty>
	inline void _CrtFreeS(_In_ _Ty **block)
	{
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");
		_freea(const_cast<void*>(void_ptr(*block)));
		*block = nullptr;
	}

	/* Deletes a pointer and sets it to NULL. */
	template <typename _Ty>
	inline void _CrtDelete(_In_ _Ty **block)
	{
		LOG_THROW_IF(*block == nullptr, "Attempting to free nullptr!");
		delete *block;
		*block = nullptr;
	}
}