#ifndef FILE_DEF_H
#define FILE_DEF_H


#define DIR_SLASH '/'
#define DIR_SLASH_STR "/"

#define IS_DIR_SLASH(c) ((c) == '/')
#define IS_ABSOLUTE_PATH(name)  (IS_DIR_SLASH((name)[0]))

#define EXT_DSO    ".so"
#define EXT_EXE    ""
#define EXT_OBJ    ".o"
#define EXT_LIB    ".a"

#define FN_DSO(n)  n EXT_DSO
#define FN_EXE(n)  n EXT_EXE
#define FN_OBJ(n)  n EXT_OBJ
#define FN_LIB(n)  n EXT_LIB

#endif
