/**
 * memory.cc - Contains stub definition of C++ new/delete operators.
 *
 * These definitions are intended to be used for testing and are weak symbols
 * to allow them to be replaced by definitions from a STL implementation.
 * These versions simply wrap malloc() and free(), they do not provide a
 * C++-specific allocator.
 */

#include <stddef.h>
#include <stdlib.h>
#include "stdexcept.h"

namespace std
{
	struct nothrow_t {};
}


/// The type of the function called when allocation fails.
typedef void (*new_handler)();
/**
 * The function to call when allocation fails.  By default, there is no
 * handler and a bad allocation exception is thrown if an allocation fails.
 */
static new_handler new_handl;
static int new_handler_lock;

// Serialize both operations and copy the callback before releasing the lock.
// In particular, never hold it while invoking a user new-handler.
static new_handler access_new_handler(new_handler handler, bool replace)
{
    while (!__sync_bool_compare_and_swap(&new_handler_lock, 0, 1)) { }
    // Both active backends support boolean CAS and explicit barriers.
    __sync_synchronize();
    new_handler previous = new_handl;
    if (replace) new_handl = handler;
    __sync_synchronize();
    (void)__sync_bool_compare_and_swap(&new_handler_lock, 1, 0);
    return previous;
}

namespace std
{
	/**
	 * Sets a function to be called when there is a failure in new.
	 */
	__attribute__((weak))
	new_handler set_new_handler(new_handler handler)
	{
		return access_new_handler(handler, true);
	}
    __attribute__((weak))
    new_handler get_new_handler() throw()
    {
        return access_new_handler(0, false);
    }
}


__attribute__((weak))
void* operator new(size_t size)
{
	void * mem = malloc(size);
	while (0 == mem)
	{
        new_handler handler = std::get_new_handler();
		if (0 != handler)
		{
			handler();
		}
		else
		{
			throw std::bad_alloc();
		}
		mem = malloc(size);
	}

	return mem;
}

__attribute__((weak))
void* operator new(size_t size, const std::nothrow_t &) throw()
{
	void *mem = malloc(size);
	while (0 == mem)
	{
        new_handler handler = std::get_new_handler();
		if (0 != handler)
		{
			try
			{
				handler();
			}
			catch (...)
			{
				// nothrow operator new should return NULL in case of
				// std::bad_alloc exception in new handler
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
		mem = malloc(size);
	}

	return mem;
}


__attribute__((weak))
void operator delete(void * ptr)
{
	free(ptr);
}


__attribute__((weak))
void * operator new[](size_t size)
{
	return ::operator new(size);
}


__attribute__((weak))
void operator delete[](void * ptr)
{
	::operator delete(ptr);
}


// C++14 sized deallocation preserves replacements of the unsized operators.
__attribute__((weak))
void operator delete(void *ptr, size_t) throw()
{
    ::operator delete(ptr);
}

__attribute__((weak))
void operator delete[](void *ptr, size_t) throw()
{
    ::operator delete[](ptr);
}
