#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#ifdef _WIN32
# undef __MSVCRT_VERSION__
# define __MSVCRT_VERSION__ 0x0800
# include <fcntl.h>
#endif

//#define WIDE_DIRTRAV 1
//#undef WIDE_DIRTRAV

#ifdef WIDE_DIRTRAV
# include "dirtravw.h"
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRPRINTF wprintf
# define DIRFPRINTF fwprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtravw_##fn
# define DIRCMP strcmp
# ifdef _WIN32
#  define DIRCASECMP wcsicmp
# else
#  define DIRCASECMP wcscasecmp
# endif
# define DIRTOUPPER towupper
# define DIRTOLOWER towlower
#else
# include "dirtrav.h"
# define DIRCHAR char
# define DIRPRINTF printf
# define DIRFPRINTF fprintf
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
# define DIRCMP strcmp
# ifdef _WIN32
#  define DIRCASECMP stricmp
# else
#  define DIRCASECMP strcasecmp
# endif
# define DIRTOUPPER toupper
# define DIRTOLOWER tolower
#endif

struct folder_data_struct {
  int opt_elevated;
  int opt_recurse;
  unsigned int level;
  uint64_t count_folders;
  uint64_t count_files;
  uint64_t total_size;
  unsigned int deepest_level;
  time_t oldest_timestamp;
  time_t newest_timestamp;
};

int file_callback (DIRTRAVFN(entry) info)
{
  time_t t;
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->count_files++;
  data->total_size += DIRTRAVFN(prop_get_size)(info);
  t = DIRTRAVFN(prop_get_modify_time)(info);
  if (t != 0) {
    if (t < data->oldest_timestamp || data->oldest_timestamp == 0)
      data->oldest_timestamp = t;
    if (t > data->newest_timestamp)
      data->newest_timestamp = t;
  }
  //DIRPRINTF(DIRTEXT("%16" PRIu64 " bytes\n"), DIRTRAVFN(prop_get_size)(info));/////
  //DIRPRINTF(DIRTEXT("%s\n"), DIRTRAVFN(prop_get_relative_path)(info));/////
  return 0;
}

int folder_callback_before (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->count_folders++;
  data->level++;
  if (data->level > data->deepest_level)
    data->deepest_level = data->level;
  return (data->opt_recurse ? 0 : -1);
}

int folder_callback_after (DIRTRAVFN(entry) info)
{
  struct folder_data_struct* data = (struct folder_data_struct*)info->callbackdata;
  data->level--;
  //if (data->level == 0)
  //  DIRPRINTF(DIRTEXT("\t%" PRIu64 " bytes in %" PRIu64 " files and %" PRIu64 " folders\n"), data->total_size, data->count_files, info->count_folders);
  //DIRPRINTF(DIRTEXT("%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\r"), data->count_files, data->count_folders, data->total_size);/////
  return 0;
}

void show_help ()
{
  DIRPRINTF(DIRTEXT(
    "Usage:  folderstats [-h] [-r] path ...\n"
    "Parameters:\n"
    "  -h          \tdisplay command line help\n"
    "  -r          \trecurse into subdirectories\n"
    "  -e          \tuse elevated privileges (only on Windows)\n"
    "  path        \tpath folder to be analyzed\n"
    "Description:\n"
    "Gets folder statistics.\n"
    "Version: %s\n"
    "\n"), DIRTRAVFN(get_version_string)()
  );
}

int parse_command_line (int argc, DIRCHAR** argv, struct folder_data_struct* data)
{
  int i;
  //process command line parameters
  if (argc <= 1) {
    show_help();
    return 0;
  }
  for (i = 1; i < argc; i++) {
    //check for command line parameters
    if (argv[i][0] && argv[i][0] == '-') {
      switch (DIRTOLOWER(argv[i][1])) {
        case 'h' :
        case '?' :
          show_help();
          _exit(0);
        case 'r' :
          data->opt_recurse = 1;
          break;
        case 'e' :
          data->opt_elevated = 1;
          break;
        default :
          DIRFPRINTF(stderr, DIRTEXT("Invalid command line parameter: %s\n"), argv[i]);
          return 0;
      }
    } else {
      return i;
    }
  }
  return i;
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
  clock_t starttime;
  struct folder_data_struct folderdata = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  //process command line options
  folderdata.opt_recurse = 0;
  if ((i = parse_command_line(argc, argv, &folderdata)) <= 0) {
    return 1;
  }

  //elevate access (needed on Windows to see all folders)
  if (folderdata.opt_elevated) {
    const DIRCHAR* errmsg;
    if ((errmsg = DIRTRAVFN(elevate_access)()) != NULL) {
      DIRFPRINTF(stderr, DIRTEXT("Error, elevating privileges: %s\n"), errmsg);
    }
  }

  //process all folders
  while (i < argc) {
    DIRPRINTF(DIRTEXT("Reading directory from: %s\n"), argv[i]);
    //reset counters
    folderdata.level = 0;
    folderdata.count_folders = 0;
    folderdata.count_files = 0;
    folderdata.total_size = 0;
    folderdata.deepest_level = 0;
    folderdata.oldest_timestamp = 0;
    folderdata.newest_timestamp = 0;
    starttime = clock();
    //process folder
    status = DIRTRAVFN(traverse_directory)(argv[i], file_callback, folder_callback_before, folder_callback_after, &folderdata);
    //DIRPRINTF(DIRTEXT("\r\t\t\t\t\t\t\t\r"), status);/////
    if (status != 0) {
      DIRPRINTF(DIRTEXT("Error, status code %i\n"), status);
    } else {
      DIRPRINTF(DIRTEXT("Folders: %" PRIu64 "\nLevels: %u\nFiles: %" PRIu64 "\nSize: %" PRIu64 " bytes\n"), folderdata.count_folders, folderdata.deepest_level, folderdata.count_files, folderdata.total_size);
      DIRPRINTF(DIRTEXT("Elapsed time: %.3f s\n"), (float)(clock() - starttime) / CLOCKS_PER_SEC);
    }
    i++;
  }

  //clean up
#if defined(_WIN32) && defined(WIDE_DIRTRAV)
  LocalFree(argv);
#endif
  return 0;
}
