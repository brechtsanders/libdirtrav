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

#ifdef WIDE_DIRTRAV
# include <dirtravw.h>
# include <windows.h>
# include <shellapi.h>
# define DIRCHAR wchar_t
# define DIRSTRLEN wcslen
# define DIRPRINTF wprintf
# define DIRTEXT_(s) L##s
# define DIRTEXT(s) DIRTEXT_(s)
# define DIRTRAVFN(fn) dirtravw_##fn
#else
# include <dirtrav.h>
# define DIRCHAR char
# define DIRSTRLEN strlen
# define DIRPRINTF printf
# define DIRTEXT(s) s
# define DIRTRAVFN(fn) dirtrav_##fn
#endif

struct dirtrav_struct {
  size_t level;
};

struct folderdata_struct {
  size_t fullpathlen;
};

int folder_callback_before (DIRTRAVFN(entry) info)
{
  struct dirtrav_struct* data = info->callbackdata;
/*
  const DIRCHAR* fullpath = DIRTRAVFN(prop_get_path)(info);
  info->folderlocaldata = malloc(sizeof(struct folderdata_struct));
  ((struct folderdata_struct*)info->folderlocaldata)->fullpathlen = DIRSTRLEN(fullpath);
  //DIRPRINTF(DIRTEXT("%*s"), (int)(data->level * 2), DIRTEXT(""));
  //DIRPRINTF(DIRTEXT("%s\n"), fullpath + (parentfolderdata ? ((struct folderdata_struct*)parentfolderdata)->fullpathlen : 0));
  //DIRPRINTF(DIRTEXT("%s\n"), fullpath);
  //DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(data->level * 2), DIRTEXT(""), filename);
  DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(data->level * 2), DIRTEXT(""), fullpath);
*/
  data->level++;
  DIRPRINTF(DIRTEXT("%*s%s\n"), (int)(data->level * 2), DIRTEXT(""), info->filename);
  return 0;
}

int folder_callback_after (DIRTRAVFN(entry) info)
{
  struct dirtrav_struct* data = info->callbackdata;
  data->level--;
/*
  if (info->folderlocaldata) {
    DIRPRINTF(DIRTEXT("%*s%i\n"), (int)(data->level * 2), DIRTEXT(""), (int)((struct folderdata_struct*)info->folderlocaldata)->fullpathlen);
    free(info->folderlocaldata);
  }
*/
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
  struct dirtrav_struct folderdata = {0};

  if (argc < 2) {
    fprintf(stderr, "Missing argument\n");
    return 1;
  }

  DIRTRAVFN(elevate_access)();

  DIRPRINTF(DIRTEXT("%s\n"), argv[1]);

  status = DIRTRAVFN(traverse_directory)(argv[1], NULL, folder_callback_before, folder_callback_after, &folderdata);
  if (status != 0) {
    fprintf(stderr, "Aborted\n");
  }

#if defined(_WIN32) && defined(WIDE_DIRTRAV)
  LocalFree(argv);
#endif
  return 0;
}
