/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#if HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <stdio.h>              /* for fprintf() */
#include <string.h>

#include <dlfcn.h>              /* for sgidladd(), dlerror() */

#include <cmplrs/rcodes.h>

#include "defs.h"
#include "dso.h"

/*
 * WARNING:  dso is used outside of main mongoose compiler (via targ_info)
 * and should not depend on other mongoose-specific files 
 * (e.g. this is why verbose is a parameter rather than a global variable).
 */

/*
 * Assume that LD_LIBRARY_PATH has already been set up correctly.
 */
void *dso_load(const char *dso, const char *path, BOOL verbose)
{
    char *dso_path;
    void *dso_handle;

    if (path != NULL && *path != '\0') {
        int length = strlen(path);
        
        dso_path = (char *)alloca(strlen(dso) + length + 2);
        strcpy(dso_path, path);
        if (!IS_DIR_SLASH(path[length - 1])) {
            dso_path[length] = DIR_SLASH;
            dso_path[length + 1] = '\0';
        }
		strcat(dso_path, dso);

        if (verbose) {
            fprintf(stderr, "\nReplacing default %s with %s%\n", 
				    dso, dso_path);
        }
    }
    else {
        dso_path = (char *)dso;
    }

    dso_handle = (void *)dlopen(dso_path, RTLD_NOW | RTLD_GLOBAL);
    if (dso_handle == NULL) {
        fprintf(stderr, "Error loading %s: %s\n",
                        dso_path, dlerror());
    }

    if (dso_handle == NULL) {
        exit(RC_SYSTEM_ERROR);
    }

    return dso_handle;
}

void *dso_get_interface(void *handle, const char *symbol)
{
    void *function;
    
    function = (void *)dlsym(handle, symbol);
    if (function == NULL) {
        fprintf(stderr, "Can not find function %s in dll\n", symbol);
        exit(RC_SYSTEM_ERROR);
    }
    return function;
}

void dso_unload(void *handle)
{
    dlclose(handle);
}
