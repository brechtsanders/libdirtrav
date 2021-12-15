#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef WIDE_DIRTRAV
# include "dirtravw.h"
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRPRINTF_S "ls"
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtravw_##fn
#else
# include "dirtrav.h"
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRPRINTF_S "s"
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
#endif

int file_found (DIRTRAVFN(entry) info)
{
  //DIRPRINTF(DIRTEXT("%s\n"), info->fullpath);/////
  (*(uint64_t*)info->callbackdata)++;
  return 0;
}

int drive_found (DIRTRAVFN(entry) info)
{
  //DIRPRINTF(DIRTEXT("%" DIRPRINTF_S  "\n"), info->fullpath);/////
  return DIRTRAVFN(traverse_directory)(info->fullpath, file_found, NULL, NULL, info->callbackdata);
}

int main()
{
  uint64_t count = 0;
  DIRTRAVFN(elevate_access)();
  DIRTRAVFN(iterate_roots)(drive_found, &count);
  DIRPRINTF(DIRTEXT("Files found: %" PRIu64 "\n"), count);
  return 0;
}
