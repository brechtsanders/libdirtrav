#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
# undef __MSVCRT_VERSION__
# define __MSVCRT_VERSION__ 0x0800
# include <fcntl.h>
#endif

#undef WIDE_DIRTRAV
//#define WIDE_DIRTRAV 1

#ifdef WIDE_DIRTRAV
# include "dirtravw.h"
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtravw_##fn
#else
# include "dirtrav.h"
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
#endif

struct folder_data_struct {
  unsigned int level;
};

int file_callback (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  DIRPRINTF(DIRTEXT("%*c%s\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_name)(info));
  DIRPRINTF(DIRTEXT("%*c [%s]\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_parentpath)(info));
  DIRPRINTF(DIRTEXT("%*c [%s]\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_top_path)(info));
  return 0;
}

int folder_callback_before (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  DIRPRINTF(DIRTEXT("%*c<%s>  %s\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_path)(info), DIRTRAVFN(prop_get_name)(info));
  DIRPRINTF(DIRTEXT("%*c [%s]\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_top_path)(info));
  DIRPRINTF(DIRTEXT("%*c [%s]\n"), data->level * 2 + 2, DIRTEXT(' '), DIRTRAVFN(prop_get_relative_path)(info));
  data->level++;
  return 0;
}

int folder_callback_after (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
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
#endif
  int i;
  int status;
  const DIRCHAR* errmsg;
  struct folder_data_struct folderdata = {0};

  DIRPRINTF(DIRTEXT("Version %s\n"), DIRTRAVFN(get_version_string)());

  if (DIRTRAVFN(supports_elevate_access())) {
    if ((errmsg = DIRTRAVFN(elevate_access)()) != NULL) {
      DIRPRINTF(DIRTEXT("Error elevating directory access: %s\n"), errmsg);
    }
  }

  for (i = 1; i < argc; i++) {
    status = DIRTRAVFN(traverse_directory)(argv[i], file_callback, folder_callback_before, folder_callback_after, &folderdata);
    if (status == 0)
      DIRPRINTF(DIRTEXT("Folder traversed successfully\n"));
    else
      DIRPRINTF(DIRTEXT("Folder not completely traversed\n"));
  }

  folderdata.level = 0;
  DIRTRAVFN(traverse_path_parts)(DIRTEXT("C:\\TEMP\\Test\\1\\2\\3"), DIRTEXT("4\\5\\6"), folder_callback_before, folder_callback_after, &folderdata);
#if defined(_WIN32) && defined(WIDE_DIRTRAV)
  LocalFree(argv);
#endif
  return 0;
}
