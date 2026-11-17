/*###INCLUDE###<copyright.in>
 * Purpose          : Header file used to define DLL connection macros on     *
 *                    sun/linux/windows platforms.                            *
 * History          : 05/02/08 - First implementation.                        *
 *----------------------------------------------------------------------------*/
#ifndef __DLL_H__
#define __DLL_H__

#ifdef __cplusplus
extern "C" {
#endif

	/*
	 * common API for explicitly loading DLL/shared libraries
	 *
	 * DLL_GET_ERROR(_msg_)
	 *
	 *   get latest error message
	 *   modify _msg_ character pointer
	 *
	 * DLL_OPEN(_dll_handle_,_name_)
	 *
	 *   explicit load the DLL/shared library
	 *   modify _dll_handle_, return !=0 if OK
	 *
	 * DLL_GET_SYMBOL(_dll_handle_,_fct_handle_,_name_)
	 *
	 *   get symbol pointer (void* typed)
	 *   modify _fct_handle_, return !=0 if OK
	 *
	 * DLL_GET_TYPED_SYMBOL(_dll_handle_,_fct_handle_,_name_,_type_)
	 *
	 *   get symbol pointer with recast to _type_
	 *   modify _fct_handle_, return !=0 if OK
	 *
	 * DLL_CLOSE(_dll_handle_)
	 *
	 *   unload the DLL/shared library, return 0 if OK
	 *
	 */

	/*{{{ Linux */

	#ifdef __linux

		#include <dlfcn.h>

                #define PLATFORM_EXT ".so"

		#define DLL_GET_ERROR(_msg_) \
			_msg_=dlerror()

		#define DLL_OPEN(_dll_handle_,_name_) \
			(_dll_handle_=dlopen(_name_,RTLD_LAZY))

		#define DLL_GET_SYMBOL(_dll_handle_,_fct_handle_,_name_) \
			(_fct_handle_=dlsym(_dll_handle_,_name_))

		#define DLL_GET_TYPED_SYMBOL(_dll_handle_,_fct_handle_,_name_,_type_) \
			(_fct_handle_=(_type_) dlsym(_dll_handle_,_name_))

		#define DLL_CLOSE(_dll_handle_) \
			dlclose(_dll_handle_)

      #define DLL_DIR_SEPARATOR "/"

	#endif /* __linux */

	/*}}}*/

	/*{{{ Solaris */

	#ifdef __sun

		#include <dlfcn.h>

                #define PLATFORM_EXT ".so"

		#define DLL_GET_ERROR(_msg_) \
			_msg_=dlerror()

		#define DLL_OPEN(_dll_handle_,_name_) \
			(_dll_handle_=dlopen(_name_,RTLD_LAZY))

		#define DLL_GET_SYMBOL(_dll_handle_,_fct_handle_,_name_) \
			(_fct_handle_=dlsym(_dll_handle_,_name_))

		#define DLL_GET_TYPED_SYMBOL(_dll_handle_,_fct_handle_,_name_,_type_) \
			(_fct_handle_=(_type_) dlsym(_dll_handle_,_name_))

		#define DLL_CLOSE(_dll_handle_) \
			dlclose(_dll_handle_)

      #define DLL_DIR_SEPARATOR "/"

	#endif /* __sun */

	/*}}}*/

	/*{{{ HP-UX */

	#ifdef __hpux

		#include <dl.h>
		#include <errno.h>

                #define PLATFORM_EXT ".so"

		#define DLL_GET_ERROR(_msg_) \
			_msg_=strerror(errno)

		#define DLL_OPEN(_dll_handle_,_name_) \
			(_dll_handle_=shl_load(_name_,BIND_DEFERRED|DYNAMIC_PATH,0L))

		#define DLL_GET_SYMBOL(_dll_handle_,_fct_handle_,_name_) \
			(!(shl_findsym(&_dll_handle_,_name_,TYPE_UNDEFINED,&_fct_handle_)))

		#define DLL_GET_TYPED_SYMBOL(_dll_handle_,_fct_handle_,_name_,_type_) \
			(!(shl_findsym(&_dll_handle_,_name_,TYPE_UNDEFINED,&_fct_handle_)))

		#define DLL_CLOSE(_dll_handle_) \
			(shl_unload(_dll_handle_))

      #define DLL_DIR_SEPARATOR "/"

	#endif /* __hpux */

	/*}}}*/


#ifdef __cplusplus
}
#endif

#endif /* __DLL_H__ */
