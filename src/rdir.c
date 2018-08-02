#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#ifdef _WIN32
// required for _setmode
# undef __MSVCRT_VERSION__
# define __MSVCRT_VERSION__ 0x0800
# include <fcntl.h>
#endif

#ifdef WIDE_DIRTRAV
#include "dirtravw.h"
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRSTRLEN wcslen
# define DIRPRINTF wprintf
# define DIRSTRFTIME wcsftime
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtravw_##fn
#else
#include "dirtrav.h"
# define DIRCHAR char
# define DIRSTRLEN strlen
# define DIRPRINTF printf
# define DIRSTRFTIME strftime
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
#endif

struct dirtrav_struct {
  size_t level;
};

const DIRCHAR* time2str (time_t datetime)
{
  static DIRCHAR buf[20];
  const struct tm* brokendowntime = localtime(&datetime);
  DIRSTRFTIME(buf, sizeof(buf) / sizeof(DIRCHAR), DIRTEXT("%Y-%m-%d %H:%M:%S"), brokendowntime);
  return buf;
}

int file_callback (DIRTRAVFN(entry) info)
{
  struct dirtrav_struct* folderdata = (info->parentinfo)->callbackdata;
  //DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(folderdata->level * 2), DIRTEXT(""), DIRTRAVFN(prop_get_path)(info));
  DIRPRINTF(DIRTEXT("%*s%s"), (int)(folderdata->level * 2), DIRTEXT(""), DIRTRAVFN(prop_get_name)(info));
  DIRPRINTF(DIRTEXT("\t%s"), time2str(DIRTRAVFN(prop_get_create_time)(info)));
  DIRPRINTF(DIRTEXT("\t%s"), time2str(DIRTRAVFN(prop_get_modify_time)(info)));
  DIRPRINTF(DIRTEXT("%16" PRIu64 " bytes\n"), DIRTRAVFN(prop_get_size)(info));
  return 0;
}

int folder_callback_before (DIRTRAVFN(entry) info)
{
  struct dirtrav_struct* data = (struct dirtrav_struct*)info->callbackdata;
  //DIRPRINTF(DIRTEXT("%*s[%s]\n"), (int)(data->level * 2), DIRTEXT(""), DIRTRAVFN(prop_get_path)(info));
  DIRPRINTF(DIRTEXT("%*s[%s]\n"), (int)(data->level * 2), DIRTEXT(""), DIRTRAVFN(prop_get_name)(info));
  data->level++;
  return 0;
}

int folder_callback_after (DIRTRAVFN(entry) info)
{
  struct dirtrav_struct* data = (struct dirtrav_struct*)info->callbackdata;
  data->level--;
  return 0;
}

#if defined(_WIN32) && defined(WIDE_DIRTRAV)
int main ()
{
  //get command line arguments
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  //set correct output (UTF-16 for Windows)
  _setmode(_fileno(stdout), _O_U16TEXT);
#else
int main(int argc, char *argv[])
{
  //set correct output (UTF-8 for Windows)
  //_setmode(_fileno(stdout), _O_U8TEXT);
#endif
  int status;
  struct dirtrav_struct folderdata = {1};

  if (argc < 2) {
    fprintf(stderr, "Missing argument\n");
    return 1;
  }

  DIRTRAVFN(elevate_access)();

  DIRPRINTF(DIRTEXT("[%s]\n"), argv[1]);

  status = DIRTRAVFN(traverse_directory)(argv[1], file_callback, folder_callback_before, folder_callback_after, &folderdata);
  if (status != 0) {
    fprintf(stderr, "Aborted\n");
  }

#if defined(_WIN32) && defined(WIDE_DIRTRAV)
  LocalFree(argv);
#endif
  return 0;
}
