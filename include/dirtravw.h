/*****************************************************************************
Copyright (C)  2013-2018  Brecht Sanders  All Rights Reserved

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
 * @file dirtravw.h
 * @brief libdirtrav header file with main functions
 * @author Brecht Sanders
 *
 * This header file defines the functions needed to traverse directories
 * using Unicode UTF-16 wide strings (wchar_t*).
 * \sa     dirtravw_traverse_directory()
 */

#ifndef INCLUDED_TRAVERSEDIRECTORY_H
#define INCLUDED_TRAVERSEDIRECTORY_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <wchar.h>

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
 * \sa     dirtravw_get_version_string()
 */
DLL_EXPORT_DIRTRAV void dirtravw_get_version (int* pmajor, int* pminor, int* pmicro);

/*! \brief get dirtrav version string
 * \return version string
 * \sa     dirtravw_get_version()
 */
DLL_EXPORT_DIRTRAV const wchar_t* dirtravw_get_version_string ();

/*! \brief check if enabling privileged access is supported (currently only implemented on Windows)
 * \return non-zero if elevating to privileged access is supported or zero if not
*/
DLL_EXPORT_DIRTRAV int dirtravw_supports_elevate_access ();

/*! \brief enable privileged access to allow reading all files and directories (only implemented on Windows)
 * \return NULL on success or error message
*/
DLL_EXPORT_DIRTRAV const wchar_t* dirtravw_elevate_access ();

/*! \brief structure used to pass directory entry data to callback functions
*/
struct dirtravw_entry_struct {
  const wchar_t* filename;                        /**< name of the current file or folder                    */
  const wchar_t* fullpath;                        /**< full path of the current file or folder               */
  const wchar_t* parentpath;                      /**< full path of the parent folder                        */
  const struct dirtravw_entry_struct* parentinfo; /**< directory entry data of parent folder                 */
  void* callbackdata;                             /**< callback data to be used freely by callback functions */
  void* folderlocaldata;                          /**< custom folder data, defaults to NULL, allocate/free in foldercallbackbefore/foldercallbackafter */
};
/*! \brief pointer to struct dirtravw_entry_struct used to pass data to callback functions
 * \sa     struct dirtravw_entry_struct
 * \sa     dirtravw_file_callback_fn
 * \sa     dirtravw_folder_callback_fn
 * \sa     dirtravw_traverse_directory()
 */
typedef struct dirtravw_entry_struct* dirtravw_entry;

/*! \brief callback function called by dirtravw_traverse_directory for each file
 * \param  info                  file information
 * \return 0 to continue processing or non-zero to abort
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
typedef int (*dirtravw_file_callback_fn) (dirtravw_entry info);

/*! \brief callback function called by dirtravw_traverse_directory for each folder
 * \param  info                  folder information
 * \return 0 to continue processing or non-zero to abort, < 0 in foldercallbackbefore to not process subfolder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
typedef int (*dirtravw_folder_callback_fn) (dirtravw_entry info);

/*! \brief recursively traverse \p directory calling calbacks for each file and folder
 * \param  directory             full path of directory to be traversed
 * \param  filecallback          optional callback function to be called before processing each file
 * \param  foldercallbackbefore  optional callback function to be called before processing each folder
 * \param  foldercallbackafter   optional callback function to be called after processing each folder (except when foldercallbackbefore returned non-zero)
 * \param  callbackdata          optional callback data to be passed to callback functions
 * \return 0 if the entire tree was traversed or the result of the callback function that caused traversal to stop
 * \sa     dirtravw_entry
 */
DLL_EXPORT_DIRTRAV int dirtravw_traverse_directory (const wchar_t* directory, dirtravw_file_callback_fn filecallback, dirtravw_folder_callback_fn foldercallbackbefore, dirtravw_folder_callback_fn foldercallbackafter, void* callbackdata);

/*! \brief split \p path into folders calling calbacks for each folder part
 * \param  startpath             optional base path to which \p path is relative
 * \param  path                  path to analyse (relative to \p startpath if specified)
 * \param  foldercallbackbefore  optional callback function to be called before processing each folder
 * \param  foldercallbackafter   optional callback function to be called after processing each folder (except when foldercallbackbefore returned non-zero)
 * \param  callbackdata          optional callback data to be passed to callback functions
 * \return 0 if the entire tree was traversed or the result of the callback function that caused traversal to stop
 * \sa     dirtravw_file_callback_fn
 * \sa     dirtravw_folder_callback_fn
 */
DLL_EXPORT_DIRTRAV int dirtravw_traverse_path_parts (const wchar_t* startpath, const wchar_t* path, dirtravw_folder_callback_fn foldercallbackbefore, dirtravw_folder_callback_fn foldercallbackafter, void* callbackdata);

/*! \brief create folder structure \p path under \p startpath
 * \param  startpath             optional base path to which \p path is relative
 * \param  path                  path of folder structure to create (relative to \p startpath if specified)
 * \param  mode                  file permission bits to use when creating folders (not used on Windows)
 * \return 0
 * \sa     dirtravw_folder_callback_fn
 */
DLL_EXPORT_DIRTRAV int dirtravw_make_full_path (const wchar_t* startpath, const wchar_t* path, mode_t mode);

/*! \brief recursively delete \p path and all files and folders it contains
 * \param  path                  path of folder structure to delete
 * \return zero on success, non-zero on error
 * \sa     dirtrav_folder_callback_fn
 */
DLL_EXPORT_DIRTRAV int dirtravw_recursive_delete (const wchar_t* path);

/*! \brief get full path of current file or folder
 * \param  entry                 system properties of directory entry
 * \return full path of current file or folder
 */
#define dirtravw_prop_get_path(entry) (entry->fullpath)

/*! \brief get parent path
 * \param  entry                 system properties of directory entry
 * \return parent path
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
#define dirtravw_prop_get_parentpath(entry) (entry->parentpath)

/*! \brief get name of current file or folder
 * \param  entry                 system properties of directory entry
 * \return name of current file or folder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
#define dirtravw_prop_get_name(entry) (entry->filename)

/*! \brief get extension of current file
 * \param  entry                 system properties of directory entry
 * \return file extension (including the leading dot) or NULL if none
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 * \sa     dirtravw_prop_get_name()
 */
DLL_EXPORT_DIRTRAV const wchar_t* dirtravw_prop_get_extension (dirtravw_entry entry);

/*! \brief determine if current file or folder is a folder
 * \param  entry                 system properties of directory entry
 * \return non-zero if current file or folder is a folder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV int dirtravw_prop_is_directory (dirtravw_entry entry);

/*! \brief determine if current file or folder is a link
 * \param  entry                 system properties of directory entry
 * \return non-zero if current file or folder is a link
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV int dirtravw_prop_is_link (dirtravw_entry entry);

/*! \brief get size of current file (not valid for folders)
 * \param  entry                 system properties of directory entry
 * \return file size in bytes
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV uint64_t dirtravw_prop_get_size (dirtravw_entry entry);

/*! \brief get creation date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return creation date and time of current file or folder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtravw_prop_get_create_time (dirtravw_entry entry);

/*! \brief get modification date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return modification date and time of current file or folder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtravw_prop_get_modify_time (dirtravw_entry entry);

/*! \brief get last access date and time of current file or folder
 * \param  entry                 system properties of directory entry
 * \return last access date and time of current file or folder
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV time_t dirtravw_prop_get_access_time (dirtravw_entry entry);

/*! \brief get top path from where directory traversal was started
 * \param  entry                 system properties of directory entry
 * \return top path
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV const wchar_t* dirtravw_prop_get_top_path (dirtravw_entry entry);

/*! \brief get relative path (relative to top path from where directory traversal was started)
 * \param  entry                 system properties of directory entry
 * \return relative path
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 */
DLL_EXPORT_DIRTRAV const wchar_t* dirtravw_prop_get_relative_path (dirtravw_entry entry);

/*! \brief get owner name
 * \param  entry                 system properties of directory entry
 * \return name of owner (the caller must call dirtravw_free()) or NULL on error
 * \sa     dirtravw_entry
 * \sa     dirtravw_traverse_directory()
 * \sa     dirtravw_free()
 */
DLL_EXPORT_DIRTRAV wchar_t* dirtravw_prop_get_owner (dirtravw_entry entry);

/*! \brief release memory allocated by this library
 * \param  data                  pointer to memory allocated by other functions in this library
 * \sa     dirtravw_prop_get_owner()
 */
DLL_EXPORT_DIRTRAV void dirtravw_free (void* data);

#ifdef __cplusplus
}
#endif

#endif //INCLUDED_TRAVERSEDIRECTORY_H
