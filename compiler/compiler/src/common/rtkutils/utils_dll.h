
#ifndef utils_dll_h
#define utils_dll_h

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * common API for explicitly loading DLL/shared libraries
   *
   * UTILS_DLL_OPEN( _ut_dll_handle_, _ut_name_ )
   * - function
   * - explicit load the DLL/shared library
   * - modify _ut_dll_handle_, return != 0 if OK
   *
   * UTILS_DLL_GET_SYMBOL( _ut_dll_handle_, _ut_pointer_, _ut_name_ )
   * - function
   * - get symbol pointer( void* typed )
   * - modify _ut_pointer_, return != 0 if OK
   *
   * UTILS_DLL_GET_TYPED_SYMBOL( _ut_dll_handle_, _ut_pointer_, _ut_name_, _ut_type_ )
   * - function
   * - get symbol pointer with recast to _ut_type_
   * - modify _ut_pointer_, return != 0 if OK
   *
   * UTILS_DLL_CLOSE( _ut_dll_handle_ )
   * - function
   * - macro function
   * - unload the DLL/shared library, return 0 if OK
   *
   * UTILS_DLL_GET_ERROR( _ut_msg_ )
   * - procedure
   * - get latest error message
   * - modify _ut_msg_ character pointer
   *
   */

  #if defined( __sun ) || defined( __linux )

    #include <dlfcn.h>

    typedef void * utilsDllHandle;

    #define UTILS_DLL_OPEN( _ut_dll_handle_, _ut_name_ )                                      ( ( _ut_dll_handle_ ) = dlopen( _ut_name_, RTLD_LAZY ) )
    #define UTILS_DLL_GET_SYMBOL( _ut_dll_handle_, _ut_pointer_, _ut_name_ )                  ( ( _ut_pointer_ ) = dlsym( _ut_dll_handle_, _ut_name_ ) )
    #define UTILS_DLL_GET_TYPED_SYMBOL( _ut_dll_handle_, _ut_pointer_, _ut_name_, _ut_type_ ) ( ( _ut_pointer_ ) = ( _ut_type_ )dlsym( _ut_dll_handle_, _ut_name_ ) )
    #define UTILS_DLL_CLOSE( _ut_dll_handle_ )                                                dlclose( _ut_dll_handle_ )
    #define UTILS_DLL_GET_ERROR( _ut_msg_ )                                                   ( ( _ut_msg_ ) = dlerror( ) )

  #endif

#ifdef __cplusplus
}
#endif

#endif

