/* libunwind - a platform-independent unwind library

   IRIX support: locate loaded objects through rld.

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

#ifndef UNW_REMOTE_ONLY

#include <string.h>
#include <rld_interface.h>

#include "libunwind_i.h"
#include "os-irix.h"

/* IRIX 6.5's libc has no dladdr(); rld provides the same service as
   _rld_new_interface (_RLD_DLADDR, address, &Dl_info). dli_fbase is
   the address of the object's ELF header in memory.

   rld defines _rld_new_interface itself and binds references to it at
   run time, as it does for libc.so. No library defines it at link time,
   so reference it weakly: a static libeh in an executable still links,
   and if rld leaves it unbound we report no unwind info.  */
#pragma weak _rld_new_interface

static int
irix_dladdr (unsigned long ip, Dl_info *info)
{
  memset (info, 0, sizeof (*info));
  if (&_rld_new_interface == NULL)
    return 0;
  _rld_new_interface (_RLD_DLADDR, ip, info);
  return info->dli_fbase != NULL;
}

int
_Uirix_phdr_for_ip (unsigned long ip,
		    int (*callback) (struct dl_phdr_info *, size_t, void *),
		    void *data)
{
  struct dl_phdr_info dpi;
  const ElfW(Ehdr) *ehdr;
  const ElfW(Phdr) *phdr;
  ElfW(Addr) link_base = 0;
  int have_link_base = 0;
  Dl_info info;
  int i;

  if (!irix_dladdr (ip, &info))
    return 0;

  ehdr = (const ElfW(Ehdr) *) info.dli_fbase;
  if (memcmp (ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    return 0;
  phdr = (const ElfW(Phdr) *) ((const char *) ehdr + ehdr->e_phoff);

  /* The ELF header is the start of the segment that maps file offset 0.
     Its link-time address gives the load bias.  */
  for (i = 0; i < ehdr->e_phnum; i++)
    if (phdr[i].p_type == PT_LOAD && phdr[i].p_offset == 0)
      {
	link_base = phdr[i].p_vaddr;
	have_link_base = 1;
	break;
      }
  if (!have_link_base)
    return 0;

  dpi.dlpi_addr = (ElfW(Addr)) (unsigned long) ehdr - link_base;
  dpi.dlpi_name = info.dli_fname ? info.dli_fname : "";
  dpi.dlpi_phdr = phdr;
  dpi.dlpi_phnum = ehdr->e_phnum;

  Debug (15, "ip=0x%lx in %s, load bias 0x%lx\n", ip, dpi.dlpi_name,
	 (unsigned long) dpi.dlpi_addr);

  return (*callback) (&dpi, sizeof (dpi), data);
}

PROTECTED int
tdep_get_elf_image (struct elf_image *ei, pid_t pid, unw_word_t ip,
		    unsigned long *segbase, unsigned long *mapoff,
		    char *path, size_t pathlen)
{
  Dl_info info;

  /* Only the current process can be inspected this way.  */
  if (pid != getpid () || !irix_dladdr ((unsigned long) ip, &info)
      || info.dli_fname == NULL)
    return -UNW_ENOINFO;

  *segbase = (unsigned long) info.dli_fbase;
  *mapoff = 0;
  if (path)
    {
      strncpy (path, info.dli_fname, pathlen);
      if (pathlen > 0)
	path[pathlen - 1] = '\0';
    }
  return elf_map_image (ei, info.dli_fname);
}

#endif /* UNW_REMOTE_ONLY */
