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

//#undef WIDE_TRAVERSE_DIRECTORY

#ifdef WIDE_TRAVERSE_DIRECTORY
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRFN(fn) wide_##fn
# define DIRCMP strcmp
# ifdef _WIN32
#  define DIRCASECMP wcsicmp
# else
#  define DIRCASECMP wcscasecmp
# endif
#else
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRFN(fn) fn
# define DIRCMP strcmp
# ifdef _WIN32
#  define DIRCASECMP stricmp
# else
#  define DIRCASECMP strcasecmp
# endif
#endif

struct folder_data_struct {
  unsigned int level;
  uint64_t count_folders;
  uint64_t count_files;
  uint64_t totalsize;
};

int file_callback (DIRFN(traverse_directory_entry) info, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->count_files++;
  data->totalsize += DIRFN(get_direntprop_size)(info);
  if (data->level == 0)
    DIRPRINTF(DIRTEXT("%s\n"), DIRFN(get_direntprop_name)(info));
  return 0;
}

int folder_callback_before (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  if (DIRCASECMP(info->fullpath, DIRTEXT("C:\\Windows\\")) == 0) {
    DIRPRINTF(DIRTEXT("Skipping: %s\n"), DIRFN(get_direntprop_path)(info));
    return -1;
  }
  data->count_folders++;
  if (data->level == 0)
    DIRPRINTF(DIRTEXT("%s\n"), DIRFN(get_direntprop_path)(info));
  data->level++;
  return 0;
}

int folder_callback_after (DIRFN(traverse_directory_entry) info, void** folderdata, void* parentfolderdata)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->level--;
  //if (data->level == 0)
  //  DIRPRINTF(DIRTEXT("\t%" PRIu64 " bytes in %" PRIu64 " files and %" PRIu64 " folders\n"), data->totalsize, data->count_files, info->count_folders);
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
  clock_t starttime;
  struct folder_data_struct folderdata = {0, 0, 0, 0};
  DIRFN(traverse_directory_elevate_access)();
  DIRPRINTF(DIRTEXT("Reading directory from: %s\n"), argv[1]);
  starttime = clock();
  status = DIRFN(traverse_directory)(argv[1], file_callback, &folderdata, folder_callback_before, folder_callback_after, &folderdata, NULL);
  DIRPRINTF(DIRTEXT("Elapsed time: %.3f s\n"), (float)(clock() - starttime) / CLOCKS_PER_SEC);
  DIRPRINTF(DIRTEXT("Status: %i\nFolders: %" PRIu64 "\nFiles: %" PRIu64 "\nSize: %" PRIu64 " bytes\n"), status, folderdata.count_folders, folderdata.count_files, folderdata.totalsize);
#if defined(_WIN32) && defined(WIDE_TRAVERSE_DIRECTORY)
  LocalFree(argv);
#endif
  return 0;
}
