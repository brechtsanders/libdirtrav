#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <dirtrav.h>

int file_found (dirtrav_entry info)
{
  (*(uint64_t*)info->callbackdata)++;
  //printf("%s\n", info->fullpath);
  return 0;
}

int main()
{
  uint64_t count = 0;
  //try to get elevated access
  dirtrav_elevate_access();
#ifdef _WIN32
  char* p;
  char* drivelist;
  DWORD drivelistlen;
  UINT drivetype;
  if ((drivelistlen = GetLogicalDriveStringsA(0, NULL)) == 0) {
    fprintf(stderr, "Error getting available drive letters\n");
    return 1;
  }
  if ((drivelist = (char*)malloc(drivelistlen)) == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return 2;
  }
  if ((drivelistlen = GetLogicalDriveStringsA(drivelistlen, drivelist)) == 0) {
    fprintf(stderr, "Error getting available drive letters\n");
    return 3;
  }
  p = drivelist;
  while (*p) {
    drivetype = GetDriveTypeA(p);
    if (drivetype != DRIVE_NO_ROOT_DIR && drivetype != DRIVE_REMOTE) {
      printf("%s\n", p);
      printf("  type: %i\n", (int)drivetype);
      dirtrav_traverse_directory(p, file_found, NULL, NULL, &count);
    }
    while (*p)
      p++;
    p++;
  }
  free(drivelist);
#else
  dirtrav_traverse_directory("/", file_found, NULL, NULL, &count);
#endif
  printf("Files found: %" PRIu64 "\n", count);
  return 0;
}
