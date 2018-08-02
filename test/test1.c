#include "traverse_directory.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
# undef __MSVCRT_VERSION__
# define __MSVCRT_VERSION__ 0x0800
# include <fcntl.h>
#endif

//#undef WIDE_TRAVERSE_DIRECTORY

#ifdef WIDE_TRAVERSE_DIRECTORY
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRFN(fn) wide_##fn
#else
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRFN(fn) fn
#endif

struct folder_data_struct {
  unsigned int level;
};

int file_callback (DIRFN(traverse_directory_entry) info, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  DIRPRINTF(DIRTEXT("%*c%s\n"), data->level * 2 + 2, DIRTEXT(' '), DIRFN(get_direntprop_name)(info));
  DIRPRINTF(DIRTEXT("%*c [%s]\n"), data->level * 2 + 2, DIRTEXT(' '), DIRFN(get_direntprop_parentpath)(info));
  return 0;
}

int folder_callback_before (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  DIRPRINTF(DIRTEXT("%*c<%s>  %s\n"), data->level * 2 + 2, DIRTEXT(' '), DIRFN(get_direntprop_path)(info), DIRFN(get_direntprop_name)(info));
  data->level++;
  return 0;
}

int folder_callback_after (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->level--;
  return 0;
}

#if defined(_WIN32) && defined(WIDE_TRAVERSE_DIRECTORY)
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
  int status;
  const DIRCHAR* errmsg;
  struct folder_data_struct folderdata = {0};

  DIRPRINTF(DIRTEXT("Version %s\n"), DIRFN(traverse_directory_get_version_string)());

  if ((errmsg = DIRFN(traverse_directory_elevate_access)()) != NULL) {
    DIRPRINTF(DIRTEXT("Error elevating directory access: %s\n"), errmsg);
  }

  status = DIRFN(traverse_directory)(argv[1], file_callback, &folderdata, folder_callback_before, folder_callback_after, &folderdata, NULL);
  if (status == 0)
    DIRPRINTF(DIRTEXT("Folder traversed successfully\n"));
  else
    DIRPRINTF(DIRTEXT("Folder not completely traversed\n"));

  folderdata.level = 0;
  DIRFN(traverse_path_parts)(DIRTEXT("C:\\TEMP\\Test\\1\\2\\3"), DIRTEXT("4\\5\\6"), folder_callback_before, folder_callback_after, &folderdata, NULL);
#if defined(_WIN32) && defined(WIDE_TRAVERSE_DIRECTORY)
  LocalFree(argv);
#endif
  return 0;
}
