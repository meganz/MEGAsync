/**
 * @file mega/win32/megasys.h
 * @brief Mega SDK platform-specific includes (Win32)
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_WIN32_OS_H
#define MEGA_WIN32_OS_H 1

#ifdef HAVE_CONFIG_H
// platform dependent constants
#include "mega/config.h"
#endif

// FIXME: move to autoconf
#define __STDC_FORMAT_MACROS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>
#include <time.h>

#include <specstrings.h>
#include <winsock2.h>
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>

#define atoll _atoi64
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS

// FIXME: move to auto-generated file
#define MEGA_MAJOR_VERSION 2
#define MEGA_MINOR_VERSION 0
#define MEGA_MICRO_VERSION 0

#include <conio.h>

#endif
