##
# @file configure.ac
# @brief an input file for autoconf tool
#
# (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
#
# This file is part of the MEGA SDK - Client Access Engine.
#
# Applications using the MEGA API must present a valid application key
# and comply with the the rules set forth in the Terms of Service.
#
# The MEGA SDK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# @copyright Simplified (2-clause) BSD License.
#
# You should have received a copy of the license along with this
# program.
##

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_include([m4/ax_prog_doxygen.m4])

AC_PREREQ([2.61])

# The Mega SDK version number is generated into config.h.
# The version in Git should reflect the *next* version planned.
m4_define([mega_major_version], [0])
m4_define([mega_minor_version], [2])
m4_define([mega_micro_version], [3])
m4_define([mega_version],
          [mega_major_version.mega_minor_version.mega_micro_version])

# libtool interface versioning
m4_define([mega_lt_revision], [0])
m4_define([mega_lt_current], [m4_eval(100 * mega_minor_version + mega_micro_version)])
m4_define([mega_lt_age], [0])

AC_INIT([libmega], [mega_version], [https://github.com/megaprivacy])

AM_INIT_AUTOMAKE([1.11 foreign silent-rules])
AC_CONFIG_HEADERS([include/mega/config.h])
LT_INIT([shared static win32-dll])
AC_CONFIG_MACRO_DIR([m4])
# enable silent build
m4_ifndef([AM_SILENT_RULES], [m4_define([AM_SILENT_RULES],[])])
AM_SILENT_RULES([yes])

MEGA_MAJOR_VERSION=mega_major_version
MEGA_MINOR_VERSION=mega_minor_version
MEGA_MICRO_VERSION=mega_micro_version

AC_SUBST(MEGA_MAJOR_VERSION)
AC_SUBST(MEGA_MINOR_VERSION)
AC_SUBST(MEGA_MICRO_VERSION)

AC_DEFINE(MEGA_MAJOR_VERSION, [mega_major_version],
	  [MEGA SDK major version.])
AC_DEFINE(MEGA_MINOR_VERSION, [mega_minor_version],
	  [MEGA SDK minor version.])
AC_DEFINE(MEGA_MICRO_VERSION, [mega_micro_version],
	  [MEGA SDK micro version.])

LT_CURRENT=mega_lt_current
LT_REVISION=mega_lt_revision
LT_AGE=mega_lt_age

AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_PID_T
AC_TYPE_OFF_T

AC_CHECK_TYPES([ssize_t])

# Check programs
AC_PROG_CXX
if test "$CXX" = no || test "$CXX:$GXX" = "g++:"; then
  AC_MSG_ERROR([C++ compiler not found !])
fi

AC_PROG_LIBTOOL
AM_SANITY_CHECK

# Check for cppcheck
AC_CHECK_PROG(HAVE_CPPCHECK, cppcheck, yes)
AM_CONDITIONAL(CPPCHECK, test -n "$HAVE_CPPCHECK")

# set C++
AC_LANG_CPLUSPLUS

# Check headers
AC_STDC_HEADERS
AC_HEADER_STDC
AC_HEADER_STDBOOL

AC_DEFINE(__STDC_CONSTANT_MACROS, [], [Force definition of constant macros for C++])
AC_DEFINE(__STDC_FORMAT_MACROS, [], [Force definition of format macros for C++])
AC_DEFINE(__STDC_LIMIT_MACROS, [], [Force definition of limit macros for C++])

# Define _GNU_SOURCE
AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions])

# Define _FILE_OFFSET_BITS and _LARGE_FILES if necessary
AC_SYS_LARGEFILE

AC_DEFINE(_GNU_SOURCE)

# Add 64-bits file support on some hosts
AC_DEFINE([_FILE_OFFSET_BITS], [64],
          [Define to 64 for large file support on some hosts])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_SIZEOF([uint64_t])
AC_STRUCT_TM
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h sys/socket.h sys/timeb.h htonl])

# Checks for inotify support.
AC_CHECK_HEADERS([sys/inotify.h mcheck.h])
AC_CHECK_FUNCS([inotify_init], [AC_DEFINE([USE_INOTIFY], [1], [Use inotify API])])

AC_CHECK_FUNCS(fdopendir sendfile clock_gettime select)

# Check for particular functions
AC_FUNC_MALLOC

# Check if building for Win32, determine Win32 API libs
AC_MSG_CHECKING([if building for Win32 platform])
case $host in
  *-*-cygwin*)
	LIBS_EXTRA="-luser32 -lkernel32"
    WIN32=yes
    ;;
  *-*-mingw*)
	LIBS_EXTRA="-lws2_32 -lole32 -lwinmm -lshlwapi"
    WIN32=yes
    ;;
  *)
	LIBS_EXTRA=""
    WIN32=no
    ;;
esac
AC_MSG_RESULT([${WIN32}])
AM_CONDITIONAL([WIN32], [test "${WIN32}" = "yes"])
AC_SUBST(LIBS_EXTRA)

SAVE_LDFLAGS=$LDFLAGS
SAVE_CXXFLAGS=$CXXFLAGS
SAVE_CPPFLAGS=$CPPFLAGS

#libcryptopp
CRYPTO_LIBS="-lcryptopp"
AC_MSG_CHECKING(for libcryptopp)
cryptopp=false
AC_ARG_WITH(cryptopp,
  [  --with-cryptopp=PATH       base of libcrypto installation],
  [AC_MSG_RESULT($with_cryptopp)
   case $with_cryptopp in
   no)
    cryptopp=false
     ;;
   yes)
    AC_CHECK_HEADERS([cryptopp/cryptlib.h],, [
        AC_MSG_ERROR([cryptopp/cryptlib.h header not found or not usable])
    ])
    AC_CHECK_LIB(cryptopp, [main], [CRYPTO_LIBS="-lcryptopp"],[
            AC_MSG_ERROR([Could not find libcryptopp])
    ])
    cryptopp=true
     ;;
   *)

if test "x$WIN32" = "xyes" ; then
    # set temp variables
    LDFLAGS="-L$with_cryptopp/cryptopp $LDFLAGS"
    CXXFLAGS="-I$with_cryptopp $CXXFLAGS"
    CPPFLAGS="-I$with_cryptopp $CPPFLAGS"

    AC_CHECK_HEADERS(cryptopp/cryptlib.h,
     EXT_LIB_PATH="-L$with_cryptopp $EXT_LIB_PATH"
     CRYPTO_CXXFLAGS="-I$with_cryptopp"
     CRYPTO_CPPFLAGS="-I$with_cryptopp"
     CRYPTO_LDFLAGS="-L$with_cryptopp/cryptopp",
     AC_MSG_ERROR([cryptopp/cryptlib.h header not found or not usable])
     )
    AC_CHECK_LIB(cryptopp, [main], [CRYPTO_LIBS="-lcryptopp"],[
            AC_MSG_ERROR([Could not find cryptopp.lib])
    ])

else
    # set temp variables
    LDFLAGS="-L$with_cryptopp/lib $LDFLAGS -pthread"
    CXXFLAGS="-I$with_cryptopp/include $CXXFLAGS"
    CPPFLAGS="-I$with_cryptopp/include $CPPFLAGS"

    AC_CHECK_HEADERS(cryptopp/cryptlib.h,
     EXT_LIB_PATH="-L$with_cryptopp/lib $EXT_LIB_PATH"
     CRYPTO_CXXFLAGS="-L$with_cryptopp/include"
     CRYPTO_CPPFLAGS="-L$with_cryptopp/include"
     CRYPTO_LDFLAGS="-L$with_cryptopp/lib",
     AC_MSG_ERROR([cryptopp/cryptlib.h header not found or not usable])
     )
    AC_CHECK_LIB(cryptopp, [main], [CRYPTO_LIBS="-lcryptopp"],[
            AC_MSG_ERROR([Could not find libcryptopp])
    ])
fi
    cryptopp=true

    #restore
    LDFLAGS=$SAVE_LDFLAGS
    CXXFLAGS=$SAVE_CXXFLAGS
    CPPFLAGS=$SAVE_CPPFLAGS
    ;;
   esac
  ],
  [AC_MSG_RESULT([--with-cryptopp not specified])
    AC_CHECK_HEADERS([cryptopp/cryptlib.h],, [
        AC_MSG_ERROR([cryptopp/cryptlib.h header not found or not usable])
    ])
    AC_CHECK_LIB(cryptopp, [main], [CRYPTO_LIBS="-lcryptopp"],[
            AC_MSG_ERROR([Could not find libcryptopp])
    ])
  ])
AC_SUBST(CRYPTO_CXXFLAGS)
AC_SUBST(CRYPTO_CPPFLAGS)
AC_SUBST(CRYPTO_LDFLAGS)
AC_SUBST(CRYPTO_LIBS)
AC_DEFINE(USE_CRYPTOPP, [1], [Define to use libcryptopp])

#
# ** DB layer **
#

# SQLite3
sqlite=false
AC_MSG_CHECKING(for SQLite)
AC_ARG_WITH(sqlite,
  [  --with-sqlite=PATH       base of SQLite installation],
  [AC_MSG_RESULT($with_sqlite)
   case $with_sqlite in
   no)
    sqlite=false
     ;;
   yes)
    AC_CHECK_HEADERS([sqlite3.h],, [
        AC_MSG_ERROR([sqlite3.h header not found or not usable])
    ])
    AC_CHECK_LIB(sqlite3, [sqlite3_open], [DB_LIBS="-lsqlite3"],[
            AC_MSG_ERROR([Could not find libsqlite3])
    ])
    sqlite=true
     ;;
   *)

if test "x$WIN32" = "xyes" ; then
    # set temp variables
    LDFLAGS="-L$with_sqlite $LDFLAGS"
    CXXFLAGS="-I$with_sqlite $CXXFLAGS"
    CPPFLAGS="-I$with_sqlite $CPPFLAGS"

    AC_CHECK_HEADERS(sqlite3.h, [
     EXT_LIB_PATH="-L$with_sqlite $EXT_LIB_PATH"
     DB_LDFLAGS="-L$with_sqlite"
     DB_CXXFLAGS="-I$with_sqlite"
     DB_CPPFLAGS="-I$with_sqlite"],
     AC_MSG_ERROR([sqlite3.h header not found or not usable])
     )
    SQLITE3_PATH=$with_sqlite
    AC_SUBST(SQLITE3_PATH)
else
    # set temp variables
    LDFLAGS="-L$with_sqlite/lib $LDFLAGS"
    CXXFLAGS="-I$with_sqlite/include $CXXFLAGS"

    AC_CHECK_HEADERS(sqlite3.h,[
     EXT_LIB_PATH="-L$with_sqlite/lib $EXT_LIB_PATH"
     DB_LDFLAGS="-L$with_sqlite/lib"
     DB_CXXFLAGS="-I$with_sqlite/include"
     DB_CXXFLAGS="-I$with_sqlite/include"],
     AC_MSG_ERROR([sqlite3.h header not found or not usable])
     )
    AC_CHECK_LIB(sqlite3, [sqlite3_open], [DB_LIBS="-lsqlite3"],[
            AC_MSG_ERROR([Could not find libsqlite3])
    ])
fi
    sqlite=true

    #restore
    LDFLAGS=$SAVE_LDFLAGS
    CXXFLAGS=$SAVE_CXXFLAGS
    CPPFLAGS=$SAVE_CPPFLAGS
    ;;
   esac
  ],
  [AC_MSG_RESULT([--with-sqlite not specified])]
  )
AM_CONDITIONAL(USE_SQLITE, test x$sqlite = xtrue)
AC_SUBST(DB_CXXFLAGS)
AC_SUBST(DB_CPPFLAGS)
AC_SUBST(DB_LDFLAGS)
AC_SUBST(DB_LIBS)

# Berkeley DB
db=false
AC_MSG_CHECKING(for Berkeley DB)
AC_ARG_WITH(db,
  [  --with-db=PATH       base of Berkeley DB installation],
  [AC_MSG_RESULT($with_db)
   case $with_db in
   no)
     db=false
     ;;
   yes)
    AC_CHECK_HEADERS([db_cxx.h],, [
        AC_MSG_ERROR([db_cxx.h header not found or not usable])
    ])

    AC_CHECK_LIB(db_cxx, [open], [DB_LIBS="-ldb_cxx"],[
            AC_MSG_ERROR([Could not find libdb_cxx])
    ])
    db=true
     ;;
   *)
    # set temp variables
    LDFLAGS="-L$with_db/lib $LDFLAGS"
    CXXFLAGS="-I$with_db/include $CXXFLAGS"

    AC_CHECK_HEADERS(db_cxx.h,
     EXT_LIB_PATH="-L$with_db/lib $EXT_LIB_PATH"
     DB_LDFLAGS="-L$with_db/lib"
     DB_CXXFLAGS="-I$with_db/include",
     AC_MSG_ERROR([db_cxx.h header not found or not usable])
     )
    AC_CHECK_LIB(db_cxx, [open], [DB_LIBS="-ldb_cxx"],[
            AC_MSG_ERROR([Could not find libdb_cxx])
    ])
    db=true

    #restore
    LDFLAGS=$SAVE_LDFLAGS
    CXXFLAGS=$SAVE_CXXFLAGS
    ;;
   esac
  ],
  [AC_MSG_RESULT([--with-db not specified])]
  )
AM_CONDITIONAL(USE_DB, test "x$db = xtrue")
AC_SUBST(DB_CXXFLAGS)
AC_SUBST(DB_LDFLAGS)
AC_SUBST(DB_LIBS)

# check if both DB layers are selected
if test "x$sqlite" = "xtrue" ; then
    if test "x$db" = "xtrue" ; then
        AC_MSG_ERROR([Please provide exactly one DB access layer, either --enable-sqlite or --enable-db.])
    fi
fi

# check if no DB layer is selected, use SQLite by the default
if test "x$sqlite" = "xfalse" ; then
    if test "x$db" = "xfalse" ; then
        AC_MSG_NOTICE([Using SQLite3 as the default DB access layer.])

        AC_CHECK_HEADERS([sqlite3.h],, [
            AC_MSG_ERROR([sqlite3.h header not found or not usable])
        ])
        AC_CHECK_LIB(sqlite3, [sqlite3_open], [DB_LIBS="-lsqlite3"],[
                AC_MSG_ERROR([Could not find libsqlite3])
        ])
        sqlite=true

        AM_CONDITIONAL(USE_SQLITE, test x$sqlite = xtrue)
        AC_SUBST(DB_CXXFLAGS)
        AC_SUBST(DB_LDFLAGS)
        AC_SUBST(DB_LIBS)
    fi
fi

if test "x$sqlite" = "xtrue" ; then
    AC_DEFINE(USE_SQLITE, [1], [Define to use SQLite])
    AC_DEFINE(USE_DB, [0], [Define to use Berkeley DB])
else
    AC_DEFINE(USE_SQLITE, [0], [Define to use SQLite])
    AC_DEFINE(USE_DB, [1], [Define to use Berkeley DB])
fi
#
# ** Posix dependent libraries **
#
if test "x$WIN32" = "xno" ; then
    # check for cURL configuration script
    AC_CHECK_PROG(curl_config_prog, curl-config, yes, no)
    if test "x$curl_config_prog" != "xyes" ; then
        AC_MSG_ERROR([Unable to find libcurl configuration program curl-config in the path.])
    fi
    LIBCURL_FLAGS=`curl-config --cflags`
    LIBCURL_LIBS=`curl-config --libs`

    AC_SUBST(LIBCURL_FLAGS)
    AC_SUBST(LIBCURL_LIBS)

    AC_MSG_CHECKING(for Curl compiled with c-ares support)
    adns=`curl-config --features | grep AsynchDNS`
    if test "x$adns" != "xAsynchDNS" ; then
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([cURL needs to be compiled with c-ares enabled!])
    fi
    AC_MSG_RESULT([yes])
fi

# determine platform include path
if test "x$WIN32" = "xyes" ; then
    platform_include="mega/win32"
else
    platform_include="mega/posix"
fi
AC_SUBST(platform_include)

# Debug
AC_ARG_ENABLE(debug,
     AS_HELP_STRING(--enable-debug, enable support for running in debug mode),
        [], [enable_debug=no])

# Enable debug flags / build
if test "x$enable_debug" = "xyes" ; then
    AM_CXXFLAGS="${AM_CXXFLAGS} -Wall -g -ggdb3 -O0"
    AC_SUBST([AM_CXXFLAGS])
fi

# Examples
AC_MSG_CHECKING([if building example applications])
AC_ARG_ENABLE([examples],
     AS_HELP_STRING([--enable-examples build example applications]),
	[], [enable_examples=no])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])
AC_MSG_RESULT([$enable_examples])

# if Examples are enables, check for specific libraries
if test "x$enable_examples" = "xyes" ; then
    # FreeImage
    AC_MSG_CHECKING(for FreeImage)
    AC_ARG_WITH(freeimage,
      [  --with-freeimage=PATH       base of FreeImage installation],
      [AC_MSG_RESULT($with_freeimage)
       case $with_freeimage in
       no)
         ;;
       yes)
        AC_CHECK_HEADERS([FreeImage.h],, [
            AC_MSG_ERROR([FreeImage.h header not found or not usable])
        ])
        AC_CHECK_LIB([freeimage], [FreeImage_Load], [FI_LIBS="-lfreeimage"], [
            AC_MSG_ERROR([FreeImage library is required for the sample client.])])

         ;;
       *)

if test "x$WIN32" = "xyes" ; then
        # set temp variables
        LDFLAGS="-L$with_freeimage/Dist $LDFLAGS"
        LDADD="-L$with_freeimage/Dist $LDADD"
        CXXFLAGS="-I$with_freeimage/Dist $CXXFLAGS"
        CPPFLAGS="-I$with_freeimage/Dist $CPPFLAGS"

        AC_CHECK_HEADERS([FreeImage.h], [
         EXT_LIB_PATH="-L$with_freeimage/Dist $EXT_LIB_PATH"
         FI_LDFLAGS="-L$with_freeimage/Dist"
         FI_LDADD="-L$with_freeimage/Dist"
         FI_CXXFLAGS="-I$with_freeimage/Dist"
         FI_CPPFLAGS="-I$with_freeimage/Dist"
         ], [
         AC_MSG_ERROR([FreeImage.h header not found or not usable])
        ])

        FI_LIBS="-lfreeimage"
else
        # set temp variables
        LDFLAGS="-L$with_freeimage/lib $LDFLAGS"
        CXXFLAGS="-I$with_freeimage/include $CXXFLAGS"
        CPPFLAGS="-I$with_freeimage/include $CPPFLAGS"

        AC_CHECK_HEADERS([FreeImage.h],[
         EXT_LIB_PATH="-L$with_freeimage/lib $EXT_LIB_PATH"
         FI_LDFLAGS="-L$with_freeimage/lib"
         FI_CXXFLAGS="-I$with_freeimage/include"
         FI_CPPFLAGS="-I$with_freeimage/include"],
         AC_MSG_ERROR([FreeImage.h header not found or not usable])
         )

        AC_CHECK_LIB([freeimage], [FreeImage_Load], [FI_LIBS="-lfreeimage"], [
            AC_MSG_ERROR([FreeImage library is required for the sample client.])])
fi
        #restore
        LDFLAGS=$SAVE_LDFLAGS
        CXXFLAGS=$SAVE_CXXFLAGS
        CPPFLAGS=$SAVE_CPPFLAGS
        ;;
       esac
      ],
      [AC_MSG_RESULT([--with-freeimage not specified])
        AC_CHECK_HEADERS([FreeImage.h],, [
            AC_MSG_ERROR([FreeImage.h header not found or not usable])
        ])
        AC_CHECK_LIB([freeimage], [FreeImage_Load], [FI_LIBS="-lfreeimage"], [
            AC_MSG_ERROR([FreeImage library is required for the sample client.])])
      ]
      )
      AC_SUBST(FI_LDFLAGS)
      AC_SUBST(FI_LIBS)
      AC_SUBST(FI_CXXFLAGS)
      AC_SUBST(FI_CPPFLAGS)

      # ReadLine
    AC_MSG_CHECKING(for Readline)
    AC_ARG_WITH(readline,
      [  --with-readline=PATH       base of Readline installation],
      [AC_MSG_RESULT($with_redline)
       case $with_readline in
       no)
         ;;
       yes)
        AC_CHECK_HEADERS([readline/readline.h],, [
            AC_MSG_ERROR([readline/readline.h header not found or not usable])
        ])
        AC_CHECK_LIB([readline], [rl_redisplay], [RL_LIBS="-lreadline"], [
            AC_MSG_ERROR([readline library is required for the sample client.])])

         ;;
       *)

if test "x$WIN32" = "xyes" ; then
        # set temp variables
        LDFLAGS="-L$with_readline/readline $LDFLAGS"
        CXXFLAGS="-I$with_readline $CXXFLAGS"
        CPPFLAGS="-I$with_readline $CPPFLAGS"

        AC_CHECK_HEADERS([readline/readline.h], [
         EXT_LIB_PATH="-L$with_readline/readline $EXT_LIB_PATH"
         RL_LDFLAGS="-L$with_readline/readline"
         RL_CXXFLAGS="-I$with_readline"],
         AC_MSG_ERROR([readline/readline.h header not found or not usable])
        )
        RL_LIBS="-lreadline"
else
        # set temp variables
        LDFLAGS="-L$with_readline/lib $LDFLAGS"
        CXXFLAGS="-I$with_readline/include $CXXFLAGS"
        CPPFLAGS="-I$with_readline/include $CPPFLAGS"
        AC_CHECK_HEADERS([readline/readline.h], [
         EXT_LIB_PATH="-L$with_readline/lib $EXT_LIB_PATH"
         RL_LDFLAGS="-L$with_readline/lib"
         RL_CXXFLAGS="-I$with_readline/include"],
         AC_MSG_ERROR([readline/readline.h header not found or not usable])
         )
        AC_CHECK_LIB([readline], [rl_redisplay], [RL_LIBS="-lreadline"], [
            AC_MSG_ERROR([readline library is required for the sample client.])])
fi
        #restore
        LDFLAGS=$SAVE_LDFLAGS
        CXXFLAGS=$SAVE_CXXFLAGS
        CPPFLAGS=$SAVE_CPPFLAGS
        ;;
       esac
      ],
      [AC_MSG_RESULT([--with-readline not specified])
        AC_CHECK_HEADERS([readline/readline.h],, [
            AC_MSG_ERROR([readline/readline.h header not found or not usable])
        ])
        AC_CHECK_LIB([readline], [rl_redisplay], [RL_LIBS="-lreadline"], [
            AC_MSG_ERROR([readline library is required for the sample client.])])
      ]
      )
      AC_SUBST(RL_LDFLAGS)
      AC_SUBST(RL_LIBS)
      AC_SUBST(RL_CXXFLAGS)

    if test "x$WIN32" = "xyes" ; then
        AC_MSG_CHECKING(for termcap)
        AC_ARG_WITH(termcap,
          [  --with-termcap=PATH       base of termcap installation],
          [AC_MSG_RESULT($with_termcap)
           case $with_termcap in
           no)
             ;;
           *)
            EXT_LIB_PATH="-L$with_termcap $EXT_LIB_PATH"
            TERMCAP_LDFLAGS="-L$with_termcap"
            TERMCAP_LIBS="-ltermcap"
            TERMCAP_CXXFLAGS="-I$with_termcap"
            TERMCAP_CPPFLAGS="-I$with_termcap"
            ;;
           esac
          ],
            AC_MSG_ERROR([termcap.h header not found or not usable])
          )
          AC_SUBST(TERMCAP_LDFLAGS)
          AC_SUBST(TERMCAP_LIBS)
          AC_SUBST(TERMCAP_CXXFLAGS)
          AC_SUBST(TERMCAP_CPPFLAGS)
    fi
fi

if test "x$WIN32" = "xyes" ; then
    AC_MSG_CHECKING(for WinHTTP)
    AC_ARG_WITH(winhttp,
      [  --with-winhttp=PATH       base of WinHTTP installation],
      [AC_MSG_RESULT($with_winhttp)
       case $with_winhttp in
       no)
         ;;
       *)
        EXT_LIB_PATH="-L$with_winhttp $EXT_LIB_PATH"
        WINHTTP_LDFLAGS="-L$with_winhttp"
        WINHTTP_LIBS="-lwinhttp"
        WINHTTP_CXXFLAGS="-I$with_winhttp"
        WINHTTP_CPPFLAGS="-I$with_winhttp"
        ;;
       esac
      ],
        AC_MSG_ERROR([WinHTTP.h header not found or not usable])
      )
      AC_SUBST(WINHTTP_LDFLAGS)
      AC_SUBST(WINHTTP_LIBS)
      AC_SUBST(WINHTTP_CXXFLAGS)
      AC_SUBST(WINHTTP_CPPFLAGS)
fi

# Tests
AC_MSG_CHECKING([if building tests])
AC_ARG_ENABLE([tests],
     AS_HELP_STRING([--enable-tests, build test applications]),
	[], [enable_tests=no])
if test "x$enable_tests" = "xyes" ; then
    AC_ARG_WITH([gtest],
        AS_HELP_STRING([--with-gtest], [specify GTest location]),
        [AC_SUBST([GTEST_DIR],[$with_gtest])],
        [AC_MSG_ERROR([GTest location must be specified])]
    )
fi
AM_CONDITIONAL([BUILD_TESTS], [test "$enable_tests" = "yes"])
AC_MSG_RESULT([$enable_tests])

dnl #########################################################################
dnl Doxygen settings
dnl #########################################################################

DX_INIT_DOXYGEN($PACKAGE_NAME, doc/Doxyfile, doc/api)
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

# Output
AC_CONFIG_FILES([
 Makefile
 include/Makefile
 libmega.pc
])
AC_OUTPUT
