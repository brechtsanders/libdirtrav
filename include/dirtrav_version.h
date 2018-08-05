/*****************************************************************************
Copyright (C)  2016  Brecht Sanders  All Rights Reserved

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
 * @file dirtrav_version.h
 * @brief XLSX I/O header file with version information.
 * @author Brecht Sanders
 *
 * Only use this header file when version information is needed at compile
 * time. Otherwise the version functions in the libraries should be used.
 * \sa     DIRTRAV_VERSION_*
 * \sa     DIRTRAV_VERSION_STRING
 * \sa     dirtrav_get_version()
 * \sa     dirtrav_get_version_string()
 */

#ifndef INCLUDED_DIRTRAV_VERSION_H
#define INCLUDED_DIRTRAV_VERSION_H

/*! \brief version number constants
 * \sa     dirtrav_get_version()
 * \sa     dirtrav_get_version_string()
 * \name   DIRTRAV_VERSION_*
 * \{
 */
/*! \brief major version number */
#define DIRTRAV_VERSION_MAJOR 0
/*! \brief minor version number */
#define DIRTRAV_VERSION_MINOR 2
/*! \brief micro version number */
#define DIRTRAV_VERSION_MICRO 1
/*! @} */

/*! \cond PRIVATE */
#define DIRTRAV_VERSION_STRINGIZE_(major, minor, micro) #major"."#minor"."#micro
#define DIRTRAV_VERSION_STRINGIZE(major, minor, micro) DIRTRAV_VERSION_STRINGIZE_(major, minor, micro)
/*! \endcond */

/*! \brief string with dotted version number \hideinitializer */
#define DIRTRAV_VERSION_STRING DIRTRAV_VERSION_STRINGIZE(DIRTRAV_VERSION_MAJOR, DIRTRAV_VERSION_MINOR, DIRTRAV_VERSION_MICRO)

/*! \brief string with name of dirtrav library */
#define DIRTRAV_NAME "libdirtrav_read"

/*! \brief string with name and version of dirtrav library \hideinitializer */
#define DIRTRAV_FULLNAME DIRTRAV_NAME " " DIRTRAV_VERSION_STRING

#endif
