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

//#define WIDE_DIRTRAV 1
#undef WIDE_DIRTRAV

#ifdef WIDE_DIRTRAV
# include "dirtravw.h"
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtraw_##fn
# define DIRCMP strcmp
# ifdef _WIN32
#  define DIRCASECMP wcsicmp
# else
#  define DIRCASECMP wcscasecmp
# endif
#else
# include "dirtrav.h"
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
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

int file_callback (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->count_files++;
  data->totalsize += DIRTRAVFN(prop_get_size)(info);
  if (data->level == 0) {
    char buf[20];
    time_t t;
    DIRPRINTF(DIRTEXT("%s\t%lu"), DIRTRAVFN(prop_get_name)(info), (unsigned long)DIRTRAVFN(prop_get_size)(info));
    t = DIRTRAVFN(prop_get_modify_time)(info);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf("\t%s", buf);
    printf("\n");
  }
  return 0;
}

int folder_callback_before (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  if (DIRCASECMP(info->fullpath, DIRTEXT("C:\\Windows\\")) == 0) {
    DIRPRINTF(DIRTEXT("Skipping: %s\n"), DIRTRAVFN(prop_get_path)(info));
    return -1;
  }
  data->count_folders++;
  if (data->level == 0)
    DIRPRINTF(DIRTEXT("%s\n"), DIRTRAVFN(prop_get_path)(info));
  data->level++;
  return 0;
}

int folder_callback_after (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->level--;
  //if (data->level == 0)
  //  DIRPRINTF(DIRTEXT("\t%" PRIu64 " bytes in %" PRIu64 " files and %" PRIu64 " folders\n"), data->totalsize, data->count_files, info->count_folders);
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
  int status;
  clock_t starttime;
  struct folder_data_struct folderdata = {0, 0, 0, 0};
  DIRTRAVFN(elevate_access)();
  DIRPRINTF(DIRTEXT("Reading directory from: %s\n"), argv[1]);
  starttime = clock();
  status = DIRTRAVFN(traverse_directory)(argv[1], file_callback, folder_callback_before, folder_callback_after, &folderdata);
  DIRPRINTF(DIRTEXT("Elapsed time: %.3f s\n"), (float)(clock() - starttime) / CLOCKS_PER_SEC);
  DIRPRINTF(DIRTEXT("Status: %i\nFolders: %" PRIu64 "\nFiles: %" PRIu64 "\nSize: %" PRIu64 " bytes\n"), status, folderdata.count_folders, folderdata.count_files, folderdata.totalsize);
#if defined(_WIN32) && defined(WIDE_DIRTRAV)
  LocalFree(argv);
#endif
  return 0;
}
