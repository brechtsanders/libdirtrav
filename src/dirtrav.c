#include "dirtrav_version.h"
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
//#include <unistd.h>
#define LOOKUP_SID
#ifdef _WIN32
#ifdef LOOKUP_SID
#define WINVER 0x0500
#include <windows.h>
#include <sddl.h>
#endif
#else
#include <sys/types.h>
#include <pwd.h>
#endif

#if defined(DIRTRAV_GENERATE)

#  include "dirtrav.h"
#  define DIRCHAR char
#  define DIRSTRLEN strlen
#  define DIRSTRCPY strcpy
#  define DIRSTRDUP strdup
#  define DIRPRINTF printf
#  define DIRTEXT(s) s
//#  define DIRFN(fn) fn
#  define DIRTRAVFN(fn) dirtrav_##fn
#  define DIRWFN(fn) fn
#  define DIR_WFN(fn) fn
#  define DIR_WUFN(fn) fn
#  define DIRWINFN(fn) fn##A
#  if !defined(_WIN32) || defined(FORCE_OPENDIR)
#    if defined(_WIN32)
//#      define STAT_FN(path,data) _stat64(path, data)
//#      define STAT_STRUCT struct __stat64
//#      define STAT_TIME_TYPE __time64_t
#      define STAT_FN(path,data) _stat(path, data)
#      define STAT_STRUCT struct _stat
#      define STAT_TIME_TYPE time_t
#    else
//#      define STAT_FN(path,data) stat64(path, data)
//#      define STAT_STRUCT struct stat64
//#      define STAT_TIME_TYPE time64_t
#      define STAT_FN(path,data) stat(path, data)
#      define STAT_STRUCT struct stat
#      define STAT_TIME_TYPE time_t
#    endif
#    define FILETIME2time_t(t) ((time_t)(t))
#  else
//#    define STAT_FN(path,data) _stat64(path, data)
//#    define STAT_STRUCT struct __stat64
//#    define STAT_TIME_TYPE FILETIME
#    define STAT_STRUCT WIN32_FIND_DATAA
#    define STAT_TIME_TYPE time_t
//#    define FILETIME2time_t(t) (STAT_TIME_TYPE)((*(__int64*)&(t) - 116444736000000000) / 10000000)
#    define FILETIME2time_t(t) (STAT_TIME_TYPE)(((__int64)(t).dwLowDateTime + ((__int64)(t).dwHighDateTime * 0x100000000) - 116444736000000000) / 10000000)
#  endif

#elif defined(DIRTRAV_GENERATE_WIDE)

#  include "dirtravw.h"
#  define DIRCHAR wchar_t
#  define DIRSTRLEN wcslen
#  define DIRSTRCPY wcscpy
#  define DIRSTRDUP wcsdup
#  define DIRPRINTF wprintf
#  define DIRTEXT_(s) L##s
#  define DIRTEXT(s) DIRTEXT_(s)
//#  define DIRFN(fn) wide_##fn
#  define DIRTRAVFN(fn) dirtravw_##fn
#  define DIRWFN(fn) w##fn
#  define DIR_WFN(fn) _w##fn
#  define DIR_WUFN(fn) _W##fn
#  define DIRWINFN(fn) fn##W
#  define STAT_FN(path,data) _wstat64(path, data)
#  if !defined(_WIN32) || defined(FORCE_OPENDIR)
#    define STAT_STRUCT struct __stat64
#    define FILETIME2time_t(t) ((time_t)(t))
#  else
#    define STAT_STRUCT WIN32_FIND_DATAW
//#    define FILETIME2time_t(t) (STAT_TIME_TYPE)((*(__int64*)&(t) - 116444736000000000) / 10000000)
#    define FILETIME2time_t(t) (STAT_TIME_TYPE)(((__int64)(t).dwLowDateTime + ((__int64)(t).dwHighDateTime * 0x100000000) - 116444736000000000) / 10000000)
#  endif
#  define STAT_TIME_TYPE time_t

#else

#  error Must define DIRTRAV_GENERATE or DIRTRAV_GENERATE_WIDE when compiling this file

#endif

#ifdef _WIN32
# include <windows.h>
# define PATH_SEPARATOR '\\'
#else
# define PATH_SEPARATOR '/'
#endif

struct DIRTRAVFN(entry_internal_struct) {
  struct DIRTRAVFN(entry_struct) external;
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  DIRWINFN(WIN32_FIND_DATA) direntry;
#else
  STAT_STRUCT statbuf;
#endif
  const DIRCHAR* toppath;   /////TO DO: consider making this public
};

DLL_EXPORT_DIRTRAV void DIRTRAVFN(get_version) (int* pmajor, int* pminor, int* pmicro)
{
  if (pmajor)
    *pmajor = DIRTRAV_VERSION_MAJOR;
  if (pminor)
    *pminor = DIRTRAV_VERSION_MINOR;
  if (pmicro)
    *pmicro = DIRTRAV_VERSION_MICRO;
}

DLL_EXPORT_DIRTRAV const DIRCHAR* DIRTRAVFN(get_version_string) ()
{
  return DIRTEXT(DIRTRAV_VERSION_STRING);
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(supports_elevate_access) ()
{
#ifdef _WIN32
  return 1;
#else
  return 0;
#endif
}

DLL_EXPORT_DIRTRAV const DIRCHAR* DIRTRAVFN(elevate_access) ()
{
#ifdef _WIN32
  //enable SE_BACKUP_NAME privilege
  TOKEN_PRIVILEGES NewState;
  LUID luid;
  HANDLE hToken = NULL;
  const DIRCHAR* result = NULL;
  //open process token for this process
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    result = DIRTEXT("Failed OpenProcessToken");
  } else {
    //get local unique ID for privilege.
    if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &luid)) {
      result = DIRTEXT("Failed LookupPrivilegeValue");
    } else {
      //adjust token privilege
      NewState.PrivilegeCount = 1;
      NewState.Privileges[0].Luid = luid;
      NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      if (!AdjustTokenPrivileges(hToken, FALSE, &NewState, 0, NULL, NULL)) {
        result = DIRTEXT("Failed AdjustTokenPrivileges");
      }
    }
    CloseHandle(hToken);
  }
  return result;
#else
  return DIRTEXT("Privilege elevation not implemented on this platform");
#endif
}

int DIRTRAVFN(iteration) (struct DIRTRAVFN(entry_internal_struct)* parentfolderinfo, DIRTRAVFN(file_callback_fn) filecallback, DIRTRAVFN(folder_callback_fn) foldercallbackbefore, DIRTRAVFN(folder_callback_fn) foldercallbackafter, void* callbackdata)
{
  int statusbefore;
  int status = 0;
  DIRCHAR* fullpath;
  struct DIRTRAVFN(entry_internal_struct) info;
  size_t filenamelen;
  const DIRCHAR* directory = parentfolderinfo->external.fullpath;
  size_t directorylen = DIRSTRLEN(directory);
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  size_t fullpathlen;
  HANDLE dir;
  DIRCHAR* searchpath;
  size_t searchpathlen;
  //determine search path
  searchpath = (DIRCHAR*)malloc((directorylen + 4 + 7) * sizeof(DIRCHAR));
  if (directorylen > 2 && directory[1] == ':') {
    memcpy(searchpath, DIRTEXT("\\\\?\\"), 4 * sizeof(DIRCHAR));
    searchpathlen = 4;
  } else {
    searchpathlen = 0;
  }
  memcpy(searchpath + searchpathlen, directory, directorylen * sizeof(DIRCHAR));
  searchpathlen += directorylen;
  if (searchpathlen > 0 && searchpath[searchpathlen - 1] != PATH_SEPARATOR)
    searchpath[searchpathlen++] = PATH_SEPARATOR;
  searchpath[searchpathlen] = '*';
  searchpath[searchpathlen + 1] = 0;
  //get directory contents
  info.external.parentpath = directory;
  info.external.parentinfo = (struct DIRTRAVFN(entry_struct)*)parentfolderinfo;
  info.external.callbackdata = callbackdata;
  info.toppath = parentfolderinfo->toppath;
#ifdef FIND_FIRST_EX_LARGE_FETCH
  if ((dir = DIRWINFN(FindFirstFileEx)(searchpath, FindExInfoBasic, &info.direntry, FindExSearchNameMatch, NULL, FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH)) != INVALID_HANDLE_VALUE) {
#else
  if ((dir = DIRWINFN(FindFirstFile)(searchpath, &info.direntry)) != INVALID_HANDLE_VALUE) {
#endif
    //process files and directories
    fullpathlen = directorylen + 1;
    fullpath = (DIRCHAR*)malloc(fullpathlen * sizeof(DIRCHAR));
    memcpy(fullpath, directory, directorylen * sizeof(DIRCHAR));
    do {
      filenamelen = DIRSTRLEN(info.direntry.cFileName);
      if (directorylen + filenamelen + 2 > fullpathlen) {
        fullpathlen = directorylen + filenamelen + 2;
        fullpath = (DIRCHAR*)realloc(fullpath, fullpathlen * sizeof(DIRCHAR));
      }
      memcpy(fullpath + directorylen, info.direntry.cFileName, filenamelen * sizeof(DIRCHAR));
      fullpath[directorylen + filenamelen] = 0;
      info.external.filename = info.direntry.cFileName;
      info.external.fullpath = fullpath; /////TO DO: use info.fullpath instead of fullpath
      if (info.direntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        //process subdirectory
        if (!(info.direntry.cFileName[0] == '.' && (info.direntry.cFileName[1] == 0 || (info.direntry.cFileName[1] == '.' && info.direntry.cFileName[2] == 0)))) {
          info.external.folderlocaldata = NULL;
          //add trailing separator
          fullpath[directorylen + filenamelen] = PATH_SEPARATOR;
          fullpath[directorylen + filenamelen + 1] = 0;
          //call callback before processing folder
          if (foldercallbackbefore)
            statusbefore = (*foldercallbackbefore)((DIRTRAVFN(entry))&info);
          else
            statusbefore = 0;
          //recurse for subdirectory
          if (statusbefore == 0)
            status = DIRTRAVFN(iteration)(&info, filecallback, foldercallbackbefore, foldercallbackafter, callbackdata);
          else if (statusbefore > 0)
            status = statusbefore;
          //call callback after processing folder
          if (foldercallbackafter && statusbefore <= 0)
            status = (*foldercallbackafter)((DIRTRAVFN(entry))&info);
        }
      } else {
        //process file
        if (filecallback) {
          status = (*filecallback)((DIRTRAVFN(entry))&info);
        }
      }
    } while (status == 0 && DIRWINFN(FindNextFile)(dir, &info.direntry));
    FindClose(dir);
    free(fullpath);
  } else {
    status = -1;
  }
  free(searchpath);
#else
  DIR_WUFN(DIR)* dir;
  struct DIR_WFN(dirent)* direntry;
  //get directory contents
  info.external.parentpath = directory;
  info.external.parentinfo = (struct DIRTRAVFN(entry_struct)*)parentfolderinfo;
  info.external.callbackdata = callbackdata;
  info.toppath = parentfolderinfo->toppath;
  if ((dir = DIR_WFN(opendir)(directory)) != NULL) {
    //process files and directories
    while (status == 0 && (direntry = DIR_WFN(readdir)(dir)) != NULL) {
      filenamelen = DIRSTRLEN(direntry->d_name);
      fullpath = (DIRCHAR*)malloc((directorylen + filenamelen + 2) * sizeof(DIRCHAR));
      memcpy(fullpath, directory, directorylen * sizeof(DIRCHAR));
      memcpy(fullpath + directorylen, direntry->d_name, filenamelen * sizeof(DIRCHAR));
      fullpath[directorylen + filenamelen] = 0;
      info.external.filename = direntry->d_name;
      info.external.fullpath = fullpath;
      if (STAT_FN(fullpath, &(info.statbuf)) == 0) {
        if (S_ISDIR(info.statbuf.st_mode)) {
          //process subdirectory
          if (!(direntry->d_name[0] == '.' && (direntry->d_name[1] == 0 || (direntry->d_name[1] == '.' && direntry->d_name[2] == 0)))) {
            info.external.folderlocaldata = NULL;
            //add trailing separator
            fullpath[directorylen + filenamelen] = PATH_SEPARATOR;
            fullpath[directorylen + filenamelen + 1] = 0;
            //call callback before processing folder
            if (foldercallbackbefore)
              statusbefore = (*foldercallbackbefore)((DIRTRAVFN(entry))&info);
            else
              statusbefore = 0;
            //recurse for subdirectory if requested
            if (statusbefore == 0)
              status = DIRTRAVFN(iteration)(&info, filecallback, foldercallbackbefore, foldercallbackafter, callbackdata);
            else if (statusbefore > 0)
              status = statusbefore;
            //call callback after processing folder
            if (foldercallbackafter && statusbefore <= 0)
              status = (*foldercallbackafter)((DIRTRAVFN(entry))&info);
          }
        } else {
          //process file
          if (filecallback) {
            info.external.filename = direntry->d_name;
            info.external.fullpath = fullpath;
            status = (*filecallback)((DIRTRAVFN(entry))&info);
          }
        }
      }
      free(fullpath);
    }
    DIR_WFN(closedir)(dir);
  } else {
    status = -1;
  }
#endif
  return status;
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(traverse_directory) (const DIRCHAR* directory, DIRTRAVFN(file_callback_fn) filecallback, DIRTRAVFN(folder_callback_fn) foldercallbackbefore, DIRTRAVFN(folder_callback_fn) foldercallbackafter, void* callbackdata)
{
  int status;
  struct DIRTRAVFN(entry_internal_struct) info;
  size_t directorylen = DIRSTRLEN(directory);
  DIRCHAR* fullpath = NULL;
  if (directorylen > 0 && directory[directorylen - 1] != PATH_SEPARATOR) {
    //add trailing separator if missing
    fullpath = (DIRCHAR*)malloc((directorylen + 2) * sizeof(DIRCHAR));
    memcpy(fullpath, directory, directorylen * sizeof(DIRCHAR));
    fullpath[directorylen] = PATH_SEPARATOR;
    fullpath[directorylen + 1] = 0;
  }
  memset(&info, 0, sizeof(info));
  //info.external.fullname = NULL;/////TO DO
  info.external.fullpath = (fullpath ? fullpath : directory);
  //info.external.parentpath = NULL;
  info.external.callbackdata = callbackdata;
  info.toppath = info.external.fullpath;
  status = DIRTRAVFN(iteration)(&info, filecallback, foldercallbackbefore, foldercallbackafter, callbackdata);
  if (fullpath)
    free(fullpath);
  return status;
}

int DIRTRAVFN(traverse_fullpath_parts_from_position) (const DIRCHAR* fullpath, size_t pos, DIRTRAVFN(folder_callback_fn) foldercallbackbefore, DIRTRAVFN(folder_callback_fn) foldercallbackafter, struct DIRTRAVFN(entry_internal_struct)* parentfolderinfo)
{
  size_t nextpos;
  struct DIRTRAVFN(entry_internal_struct) info;
  int status = 0;
  //skip initial path separator if any
  while (fullpath[pos] == PATH_SEPARATOR)
    pos++;
  //abort if no next part is found
  if (!fullpath[pos])
    return 0;
  //determine end of current part
  nextpos = pos;
  while (fullpath[nextpos] && fullpath[nextpos] != PATH_SEPARATOR)
    nextpos++;
  //recurse through parts
  DIRCHAR* currentpath;
  //determine current full path
  currentpath = (DIRCHAR*)malloc((nextpos + 1) * sizeof(DIRCHAR));
  memcpy(currentpath, fullpath, nextpos * sizeof(DIRCHAR));
  currentpath[nextpos] = 0;
  //initialize directory information structure
  info.external.filename = currentpath + pos;
  info.external.fullpath = currentpath;
  info.external.parentpath = parentfolderinfo->external.fullpath;
  info.external.parentinfo = (struct DIRTRAVFN(entry_struct)*)parentfolderinfo;
  info.external.callbackdata = parentfolderinfo->external.callbackdata;
  info.external.folderlocaldata = NULL;
  info.toppath = parentfolderinfo->toppath;
  //call callback before processing folder
  if (foldercallbackbefore && status == 0) {
    (status = (*foldercallbackbefore)((DIRTRAVFN(entry))&info));
  }
  //recurse through next parts
  if (status == 0 && fullpath[nextpos])
    status = DIRTRAVFN(traverse_fullpath_parts_from_position)(fullpath, nextpos, foldercallbackbefore, foldercallbackafter, &info);
  //call callback after processing folder
  if (foldercallbackafter) {
    if (status == 0)
      status = (*foldercallbackafter)((DIRTRAVFN(entry))&info);
    else
      (*foldercallbackafter)((DIRTRAVFN(entry))&info);
  }
  free(currentpath);
  return 0;
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(traverse_path_parts) (const DIRCHAR* startpath, const DIRCHAR* path, DIRTRAVFN(folder_callback_fn) foldercallbackbefore, DIRTRAVFN(folder_callback_fn) foldercallbackafter, void* callbackdata)
{
  size_t i;
  size_t pos;
  int status;
  DIRCHAR* fullpath;
  struct DIRTRAVFN(entry_internal_struct) info;
  DIRCHAR* startpathfixed;
  if (!path || !*path)
    return 0;
  //construct full path
  if (!startpath || !*startpath) {
    pos = 0;
    fullpath = DIRSTRDUP(path);
    startpathfixed = DIRSTRDUP(DIRTEXT(""));
  } else {
    //add trailing to start path separator if missing
    size_t startpathlen = DIRSTRLEN(startpath);
    if (startpathlen > 0 && startpath[startpathlen - 1] != PATH_SEPARATOR) {
      startpathfixed = (DIRCHAR*)malloc((startpathlen + 2) * sizeof(DIRCHAR));
      memcpy(startpathfixed, startpath, startpathlen * sizeof(DIRCHAR));
      startpathfixed[startpathlen] = PATH_SEPARATOR;
      startpathfixed[startpathlen + 1] = 0;
    } else {
      startpathfixed = DIRSTRDUP(startpath);
    }
    //determine full path
    pos = DIRSTRLEN(startpathfixed);
    fullpath = (DIRCHAR*)malloc((pos + DIRSTRLEN(path) + 2) * sizeof(DIRCHAR));
    DIRSTRCPY(fullpath, startpathfixed);
    if (pos > 0 && fullpath[pos] != PATH_SEPARATOR && path[0] != PATH_SEPARATOR)
      fullpath[pos++] = PATH_SEPARATOR;
    /////TO DO: what if startpath ends and path begins with PATH_SEPARATOR?
    DIRSTRCPY(fullpath + pos, path);
    i = DIRSTRLEN(fullpath);
    if (i > 0 && fullpath[i - 1] == PATH_SEPARATOR)
      fullpath[i - 1] = 0;
  }
  //strip double separators (except at the beginning on Windows)
  i = 0;
#ifdef _WIN32
  if (fullpath[0] == PATH_SEPARATOR && fullpath[1] == PATH_SEPARATOR) {
    i = 2;
    while (fullpath[i] == PATH_SEPARATOR)
      i++;
  }
#endif
  while (fullpath[i]) {
    if (fullpath[i] == PATH_SEPARATOR && fullpath[i + 1] == PATH_SEPARATOR) {
      memmove(fullpath + i, fullpath + i + 1, (DIRSTRLEN(fullpath + i + 1) + 1) * sizeof(DIRCHAR));
      if (pos >= i + 1)
        pos--;
    } else {
      i++;
    }
  }
  //strip . and  .. entries from path
  if (fullpath[0] == '.' && fullpath[1] == PATH_SEPARATOR) {
    memmove(fullpath, fullpath + 2, (DIRSTRLEN(fullpath + 2) + 1) * sizeof(DIRCHAR));
    if (pos >= 2)
      pos -= 2;
  }
  i = 0;
  while (fullpath[i]) {
    if (fullpath[i] == PATH_SEPARATOR && fullpath[i + 1] == '.' && fullpath[i + 2] == PATH_SEPARATOR) {
      memmove(fullpath + i, fullpath + i + 2, (DIRSTRLEN(fullpath + i + 2) + 1) * sizeof(DIRCHAR));
      if (pos >= i + 2)
        pos -= 2;
    } else if (i > 0 && fullpath[i] == PATH_SEPARATOR && fullpath[i + 1] == '.' && fullpath[i + 2] == '.' && (fullpath[i + 3] == PATH_SEPARATOR || fullpath[i + 3] == 0)) {
      int startpos = i - 1;
      while (startpos > 0 && fullpath[startpos] != PATH_SEPARATOR)
        startpos--;
      memmove(fullpath + startpos, fullpath + i + 3, (DIRSTRLEN(fullpath + i + 3) + 1) * sizeof(DIRCHAR));
      if (pos >= i + 3)
        pos -= 3 + (i - startpos);
      i = startpos;
    } else {
      i++;
    }
  }
  //initialize directory information structure
  info.external.filename = NULL;
  info.external.fullpath = fullpath;
  info.external.parentpath = NULL;
  info.external.parentinfo = NULL;
  info.external.callbackdata = callbackdata;
  info.external.folderlocaldata = NULL;
  info.toppath = startpathfixed;
  status = DIRTRAVFN(traverse_fullpath_parts_from_position)(fullpath, pos, foldercallbackbefore, foldercallbackafter, &info);
  //clean up
  free(fullpath);
  free(startpathfixed);
  return status;
}

int DIRTRAVFN(recursive_delete_file_callback) (DIRTRAVFN(entry) info)
{
#ifdef _WIN32
  if (DIRWINFN(DeleteFile)(info->fullpath))
#else
  if (unlink(info->fullpath) = 0)
#endif
    return 0;
  return 1;
}

int DIRTRAVFN(recursive_delete_folder_callback) (DIRTRAVFN(entry) info)
{
#ifdef _WIN32
  if (DIRWINFN(RemoveDirectory)(info->fullpath))
#else
  if (rmdir(info->fullpath) == 0)
#endif
    return 0;
  return 2;
}

int DIRTRAVFN(recursive_delete) (const DIRCHAR* path)
{
  int result;
  if (!path || !*path)
    return -2;
  if ((result = DIRTRAVFN(traverse_directory)(path, DIRTRAVFN(recursive_delete_file_callback), NULL, DIRTRAVFN(recursive_delete_folder_callback), NULL)) == 0) {
#ifdef _WIN32
    if (!DIRWINFN(RemoveDirectory)(path))
#else
    if (rmdir(path) != 0)
#endif
      return -1;
  }
  return result;
}

int DIRTRAVFN(make_full_path_callback) (DIRTRAVFN(entry) info)
{
#ifdef _WIN32
  DIRWINFN(CreateDirectory)(((struct DIRTRAVFN(entry_internal_struct)*)info)->external.fullpath, NULL);
#else
  DIRWFN(mkdir)(info->fullpath, *(mode_t*)info->callbackdata);
#endif
  return 0;
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(make_full_path) (const DIRCHAR* startpath, const DIRCHAR* path, mode_t mode)
{
#ifndef _WIN32
  if (mode == 0)
    mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
#endif
  return DIRTRAVFN(traverse_path_parts)(startpath, path, DIRTRAVFN(make_full_path_callback), NULL, &mode);
  /////TO DO: proper result code indicating if tree was not successfully created
}

#ifndef MAX_EXTENSION_LENGTH
#define MAX_EXTENSION_LENGTH 12
#endif

DLL_EXPORT_DIRTRAV const DIRCHAR* DIRTRAVFN(prop_get_extension) (DIRTRAVFN(entry) entry)
{
  const DIRCHAR* p;
  size_t extlen;
  const DIRCHAR* filename = entry->filename;
  if (!filename || !*filename)
    return NULL;
  p = filename + DIRSTRLEN(filename);
  extlen = 0;
  while (p-- != filename && extlen++ < MAX_EXTENSION_LENGTH) {
    if (*p == DIRTEXT('.'))
      return p;
  }
  return NULL;
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(prop_is_directory) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return (((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 1 : 0);
#else
  return (S_ISDIR(((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_mode) ? 1 : 0);
#endif
}

DLL_EXPORT_DIRTRAV int DIRTRAVFN(prop_is_link) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return (((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ? 1 : 0);
#elif defined(_WIN32)
  return 0;     /////TO DO: check if link on Windows
#else
  return (S_ISLNK(((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_mode) ? 1 : 0);
#endif
}

DLL_EXPORT_DIRTRAV uint64_t DIRTRAVFN(prop_get_size) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return (uint64_t)0x100000000 * ((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.nFileSizeHigh + ((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.nFileSizeLow;
#else
  return ((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_size;
#endif
}

DLL_EXPORT_DIRTRAV time_t DIRTRAVFN(prop_get_create_time) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.ftCreationTime);
#else
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_ctime);
#endif
}

DLL_EXPORT_DIRTRAV time_t DIRTRAVFN(prop_get_modify_time) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.ftLastWriteTime);
#else
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_mtime);
#endif
}

DLL_EXPORT_DIRTRAV time_t DIRTRAVFN(prop_get_access_time) (DIRTRAVFN(entry) entry)
{
#if defined(_WIN32) && !defined(FORCE_OPENDIR)
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->direntry.ftLastAccessTime);
#else
  return FILETIME2time_t(((struct DIRTRAVFN(entry_internal_struct)*)entry)->statbuf.st_atime);
#endif
}

DLL_EXPORT_DIRTRAV const DIRCHAR* DIRTRAVFN(prop_get_top_path) (DIRTRAVFN(entry) entry)
{
  return ((struct DIRTRAVFN(entry_internal_struct)*)entry)->toppath;
}

DLL_EXPORT_DIRTRAV const DIRCHAR* DIRTRAVFN(prop_get_relative_path) (DIRTRAVFN(entry) entry)
{
  size_t toppathlen;
  toppathlen = DIRSTRLEN(((struct DIRTRAVFN(entry_internal_struct)*)entry)->toppath);
  return entry->fullpath + toppathlen;
}

DLL_EXPORT_DIRTRAV DIRCHAR* DIRTRAVFN(prop_get_owner) (DIRTRAVFN(entry) entry)
{
  DIRCHAR* result = NULL;
#ifdef _WIN32
  SECURITY_DESCRIPTOR* secdes;
  PSID ownersid;
  BOOL ownderdefaulted;
  DWORD len = 0;
  DIRWINFN(GetFileSecurity)(entry->fullpath, OWNER_SECURITY_INFORMATION, NULL, 0, &len);
  if (len > 0) {
    secdes = (SECURITY_DESCRIPTOR*)malloc(len);
    if (DIRWINFN(GetFileSecurity)(entry->fullpath, OWNER_SECURITY_INFORMATION, secdes, len, &len)) {
      if (GetSecurityDescriptorOwner(secdes, &ownersid, &ownderdefaulted)) {
        SID_NAME_USE accounttype;
        DIRCHAR* name;
        DIRCHAR* domain;
        DWORD domainlen;
        len = 0;
        domainlen = 0;
        if (DIRWINFN(LookupAccountSid)(NULL, ownersid, NULL, &len, NULL, &domainlen, &accounttype) && len > 0 && domainlen > 0) {
          name = (DIRCHAR*)malloc(len * sizeof(DIRCHAR));
          domain = (DIRCHAR*)malloc(domainlen * sizeof(DIRCHAR));
          if (DIRWINFN(LookupAccountSid)(NULL, ownersid, name, &len, domain, &domainlen, &accounttype)) {
            result = (DIRCHAR*)malloc((domainlen + len + 2) * sizeof(DIRCHAR));
            memcpy(result, domain, domainlen * sizeof(DIRCHAR));
            result[domainlen] = '\\';
            DIRSTRCPY(result + domainlen + 1, name);
          }
          free(name);
          free(domain);
        }
#ifdef LOOKUP_SID
        if (!result) {
          DIRCHAR* sidstring;
          if (DIRWINFN(ConvertSidToStringSid)(ownersid, &sidstring)) {
            result = DIRSTRDUP(sidstring);
            LocalFree(sidstring);
          }
        }
#endif
      }
    }
    free(secdes);
  }
  /////TO DO: GROUP_SECURITY_INFORMATION
  /////TO DO: query remote server to look up users on network drive - to get volume information about path, see GetVolumeNameForVolumeMountPoint()
#else
  struct stat fileinfo;
  if (stat(entry->fullpath, &fileinfo) == 0) {
    struct passwd* pw = getpwuid(fileinfo.st_uid);
    if (pw) {
      result = strdup(pw->pw_name);
    }
    //struct group* gr = getgrgid(fileinfo.st_gid);
  }
#endif
  return result;
}

DLL_EXPORT_DIRTRAV void DIRTRAVFN(free) (void* data)
{
  free(data);
}

#undef DIRTRAV_GENERATE
#undef DIRTRAV_GENERATE_WIDE
#undef DIRCHAR
#undef DIRSTRLEN
#undef DIRSTRCPY
#undef DIRSTRDUP
#undef DIRPRINTF
#undef DIRTEXT_
#undef DIRTEXT
#undef DIRFN
#undef DIRWFN
#undef DIR_WFN
#undef DIR_WUFN
#undef DIRWINFN
#undef STAT_FN
#undef STAT_STRUCT
#undef STAT_TIME_TYPE
#undef FILETIME2time_t
