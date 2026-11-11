/* libunwind - a platform-independent unwind library

   IRIX support: IRIX has neither <link.h> nor dl_iterate_phdr. This
   header supplies the parts of <link.h> that libunwind uses, and a
   lookup that reports the one loaded object containing a given address.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef os_irix_h
#define os_irix_h

#include <stddef.h>
#include <elf.h>

#if _MIPS_SIM == _ABI64
# define ElfW(type)	Elf64_##type
#else
# define ElfW(type)	Elf32_##type
#endif

#ifndef PT_GNU_EH_FRAME
# define PT_GNU_EH_FRAME	0x6474e550
#endif

/* The subset of glibc's struct dl_phdr_info that libunwind reads.  */
struct dl_phdr_info
  {
    ElfW(Addr) dlpi_addr;		/* load bias of the object */
    const char *dlpi_name;
    const ElfW(Phdr) *dlpi_phdr;
    ElfW(Half) dlpi_phnum;
  };

/* Find the loaded object (executable or DSO) that contains IP and call
   CALLBACK once for it, as dl_iterate_phdr would. Returns CALLBACK's
   result, or 0 if no object contains IP.  */
extern int _Uirix_phdr_for_ip (unsigned long ip,
			       int (*callback) (struct dl_phdr_info *,
						size_t, void *),
			       void *data);

#endif /* os_irix_h */
