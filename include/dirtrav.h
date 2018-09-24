/*****************************************************************************
Copyright (C)  2018  Brecht Sanders  All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

/**
 * @file dirtrav.h
 * @brief libdirtrav header file with main functions
 * @author Brecht Sanders
 *
 * This header file defines the functions needed to traverse directories.
 * \sa     dirtrav_traverse_directory()
 */

#ifndef INCLUDED_TRAVERSEDIRECTORY_H
#define INCLUDED_TRAVERSEDIRECTORY_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

/*! \cond PRIVATE */
#if !defined(DLL_EXPORT_DIRTRAV)
# if defined(_WIN32) && defined(BUILD_DIRTRAV_DLL)
#  define DLL_EXPORT_DIRTRAV __declspec(dllexport)
# elif defined(_WIN32) && !defined(STATIC) && !defined(BUILD_DIRTRAV_STATIC) && !defined(BUILD_DIRTRAV)
#  define DLL_EXPORT_DIRTRAV __declspec(dllimport)
# else
#  define DLL_EXPORT_DIRTRAV
# endif
#endif
/*! \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief get dirtrav version string
 * \param  pmajor        pointer to integer that will receive major version number
 * \param  pminor        pointer to integer that will receive minor version number
 * \param  pmicro        pointer to integer that will receive micro version number
 * \sa     dirtrav_get_version_string()
 */
DLL_EXPORT_DIRTRAV void dirtrav_get_version (int* pmajor, int* pminor, int* pmicro);

/*! \brief get dirtrav version string
 * \return version string
 * \sa     dirtrav_get_version()
 */
DLL_EXPORT_DIRTRAV const char* dirtrav_get_version_string ();

/*! \brief enable privileged access to allow reading all files and directories (only implemented on Windows)
 * \return NULL on success or error message
*/
DLL_EXPORT_DIRTRAV const char* dirtrav_elevate_access ();

/*! \brief structure used to pass directory entry data to callback functions
*/
struct dirtrav_entry_struct {
  const char* filename;                           /**< name of the current file or folder                    */
  const char* fullpath;                           /**< full path of the current file or folder               */
  const char* parentpath;                         /**< full path of the parent folder                        */
  const struct dirtrav_entry_struct* parentinfo;  /**< directory entry data of parent folder                 */
  void* callbackdata;                             /**< callback data to be used freely by callback functions */
  void* folderlocaldata;                          /**< custom folder data, defaults to NULL, allocate/free in foldercallbackbefore/foldercallbackafter */
};
/*! \brief pointer to struct dirtravw_entry_struct used to pass data to callback functions
 * \sa     struct dirtrav_entry_struct
 * \sa     dirtrav_file_callback_fn
 * \sa     dirtrav_folder_callback_fn
 * \sa     dirtrav_traverse_directory()
 */
typedef struct dirtrav_entry_struct* dirtrav_entry;

/*! \brief callback function called by dirtrav_traverse_directory for each file
 * \param  info                  file information
 * \return 0 to continue processing or non-zero to abort
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
typedef int (*dirtrav_file_callback_fn) (dirtrav_entry info);

/*! \brief callback function called by dirtrav_traverse_directory for each folder
 * \param  info                  folder information
 * \return 0 to continue processing or non-zero to abort, < 0 in foldercallbackbefore to not process subfolder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
typedef int (*dirtrav_folder_callback_fn) (dirtrav_entry info);

/*! \brief recursively traverse \p directory calling calbacks for each file and folder
 * \param  directory             full path of directory to be traversed
 * \param  filecallback          optional callback function to be called before processing each file
 * \param  foldercallbackbefore  optional callback function to be called before processing each folder
 * \param  foldercallbackafter   optional callback function to be called after processing each folder (except when foldercallbackbefore returned non-zero)
 * \param  callbackdata          optional callback data to be passed to callback functions
 * \return 0 if the entire tree was traversed or the result of the callback function that caused traversal to stop
 * \sa     dirtrav_entry
 */
DLL_EXPORT_DIRTRAV int dirtrav_traverse_directory (const char* directory, dirtrav_file_callback_fn filecallback, dirtrav_folder_callback_fn foldercallbackbefore, dirtrav_folder_callback_fn foldercallbackafter, void* callbackdata);

/*! \brief split \p path into folders calling calbacks for each folder part
 * \param  startpath             optional base path to which \p path is relative
 * \param  path                  path to analyse
 * \param  foldercallbackbefore  optional callback function to be called before processing each folder
 * \param  foldercallbackafter   optional callback function to be called after processing each folder (except when foldercallbackbefore returned non-zero)
 * \param  callbackdata          optional callback data to be passed to callback functions
 * \return 0 if the entire tree was traversed or the result of the callback function that caused traversal to stop
 * \sa     dirtrav_file_callback_fn
 * \sa     dirtrav_folder_callback_fn
 */
DLL_EXPORT_DIRTRAV int dirtrav_traverse_path_parts (const char* startpath, const char* path, dirtrav_folder_callback_fn foldercallbackbefore, dirtrav_folder_callback_fn foldercallbackafter, void* callbackdata);

/*! \brief create folder structure \p path under \p startpath
 * \param  startpath             optional base path to which path is relative
 * \param  path                  path of folder structure to create under \p startpath
 * \param  mode                  file permission bits to use when creating folders (not used on Windows)
 * \return 0
 * \sa     dirtrav_folder_callback_fn
 */
DLL_EXPORT_DIRTRAV int dirtrav_make_full_path (const char* startpath, const char* path, mode_t mode);

/*! \brief get full path of current file or folder
 * \param  entry                 system properties of directory entry
 * \return full path of current file or folder
 */
#define dirtrav_prop_get_path(entry) (entry->fullpath)

/*! \brief get parent path
 * \param  entry                 system properties of directory entry
 * \return parent path
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
#define dirtrav_prop_get_parentpath(entry) (entry->parentpath)

/*! \brief get name of current file or folder
 * \param  entry                 system properties of directory entry
 * \return name of current file or folder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
#define dirtrav_prop_get_name(entry) (entry->filename)

/*! \brief get extension of current file
 * \param  entry                 system properties of directory entry
 * \return file extension (including the leading dot) or NULL if none
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 * \sa     dirtrav_prop_get_name()
 */
DLL_EXPORT_DIRTRAV const char* dirtrav_prop_get_extension (dirtrav_entry entry);

/*! \brief determine if current file or folder is a folder
 * \param  entry                 system properties of directory entry
 * \return non-zero if current file or folder is a folder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV int dirtrav_prop_is_directory (dirtrav_entry entry);

/*! \brief determine if current file or folder is a link
 * \param  entry                 system properties of directory entry
 * \return non-zero if current file or folder is a link
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV int dirtrav_prop_is_link (dirtrav_entry entry);

/*! \brief get size of current file (not valid for folders)
 * \param  entry                 system properties of directory entry
 * \return file size in bytes
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV uint64_t dirtrav_prop_get_size (dirtrav_entry entry);

/*! \brief get creation date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return creation date and time of current file or folder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtrav_prop_get_create_time (dirtrav_entry entry);

/*! \brief get modification date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return modification date and time of current file or folder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtrav_prop_get_modify_time (dirtrav_entry entry);

/*! \brief get last access date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return last access date and time of current file or folder
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtrav_prop_get_access_time (dirtrav_entry entry);

/*! \brief get top path from where directory traversal was started
 * \param  entry                 system properties of directory entry
 * \return top path
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV const char* dirtrav_prop_get_top_path (dirtrav_entry entry);

/*! \brief get relative path (relative to top path from where directory traversal was started)
 * \param  entry                 system properties of directory entry
 * \return relative path
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV const char* dirtrav_prop_get_relative_path (dirtrav_entry entry);

/*! \brief get owner name
 * \param  entry                 system properties of directory entry
 * \return name of owner (the caller must call free()) or NULL on error
 * \sa     dirtrav_entry
 * \sa     dirtrav_traverse_directory()
 */
DLL_EXPORT_DIRTRAV char* dirtrav_prop_get_owner (dirtrav_entry entry);

#ifdef __cplusplus
}
#endif

#endif //INCLUDED_TRAVERSEDIRECTORY_H
