#pragma once
#include <cstdlib>
#include <malloc.h>
#include "Logging.h"

/* Allocates C-style memory on the heap. */
#define malloc_s(type, size)			reinterpret_cast<type*>(malloc((size) * sizeof(type)))
/* Allocates C-style memory on the stack. */
#define malloca_s(type, size)			reinterpret_cast<type*>(_malloca((size) * sizeof(type)))
/* Re-allocates C-style memory on the heap. */
#define realloc_s(type, block, size)	reinterpret_cast<type*>(realloc((block), (size) * sizeof(type)))
/* Allocates a C-style array. */
#define calloc_s(type, size)			reinterpret_cast<type*>(calloc((size), sizeof(type))
/* Converts the specified pointer to a void pointer. */
#define void_ptr(ptr)					reinterpret_cast<const void*>(ptr)
/* Gets the byte offset to the specified member in the specified type as a void pointer. */
#define offset_ptr(s, m)				void_ptr(offsetof(s, m))

#if defined(DEBUG)
/* Deletes a C-style memory block and sets it to NULL. */
#define free_s(block)					{ if ((block) != nullptr) { free(block); (block) = nullptr; } else ASSERT("Attempting to free nullptr!", "Cannot free nullptr!"); }
/* Deletes a const C-style memory block and sets it to NULL. */
#define free_c_s(block, type)			{ if ((block) != nullptr) { free(const_cast<type*>(block); (block) = nullptr; } else ASSERT("Attempting to free nullptr!", "Cannot free nullptr!"); }
/* Deletes a pointer and sets it to NULL. */
#define delete_s(block)					{ if ((block) != nullptr) { delete (block); (block) = nullptr; } else ASSERT("Attempting to delete nullptr!", "Cannot delete nullptr!"); }
#else
/* Deletes a C-style memory block. */
#define free_s(block)					free(block)
/* Deletes a const C-style memory block. */
#define free_c_s(block, type)			free(const_cast<type*>(block))
/* Deletes a pointer. */
#define delete_s(block)					delete block
#endif

/* Deletes a const C-style string (sets to NULL on debug mode). */
#define free_cstr_s(str)				free_c_s(str, char)		