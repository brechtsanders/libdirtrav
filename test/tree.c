#include "traverse_directory.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#ifdef _WIN32
# undef __MSVCRT_VERSION__
# define __MSVCRT_VERSION__ 0x0800
# include <fcntl.h>
#endif

#undef WIDE_TRAVERSE_DIRECTORY

#ifdef WIDE_TRAVERSE_DIRECTORY
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRSTRLEN wcslen
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRFN(fn) wide_##fn
#else
# define DIRCHAR char
# define DIRSTRLEN strlen
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRFN(fn) fn
#endif

struct traverse_directory_struct {
  size_t level;
};

struct folderdata_struct {
  size_t fullpathlen;
};

int folder_callback_before (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct traverse_directory_struct* data = (struct traverse_directory_struct*)info->callbackdata;
  const DIRCHAR* fullpath = DIRFN(get_direntprop_path)(info);
  *folderdata = malloc(sizeof(struct folderdata_struct));
  ((struct folderdata_struct*)*folderdata)->fullpathlen = DIRSTRLEN(fullpath);
  //DIRPRINTF(DIRTEXT("%*s"), (int)(data->level * 2), DIRTEXT(""));
  //DIRPRINTF(DIRTEXT("%s\n"), fullpath + (parentfolderdata ? ((struct folderdata_struct*)parentfolderdata)->fullpathlen : 0));
  //DIRPRINTF(DIRTEXT("%s\n"), fullpath);
  //DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(data->level * 2), DIRTEXT(""), filename);
  DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(data->level * 2), DIRTEXT(""), fullpath);
  data->level++;
  return 0;
}

int folder_callback_after (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct traverse_directory_struct* data = (struct traverse_directory_struct*)info->callbackdata;
  data->level--;
  if (*folderdata)
    free(*folderdata);
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
  struct traverse_directory_struct folderdata = {0};

  if (argc < 2) {
    fprintf(stderr, "Missing argument\n");
    return 1;
  }

  DIRFN(traverse_directory_elevate_access)();

  status = DIRFN(traverse_directory)(argv[1], NULL/*file_callback*/, &folderdata, folder_callback_before, folder_callback_after, &folderdata, NULL);
  if (status != 0) {
    fprintf(stderr, "Aborted\n");
  }

#if defined(_WIN32) && defined(WIDE_TRAVERSE_DIRECTORY)
  LocalFree(argv);
#endif
  return 0;
}
