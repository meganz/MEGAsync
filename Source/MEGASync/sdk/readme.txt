Introduction
============

This is megaclient, a command-line client utility for accessing the MEGA
secure cloud storage service. For more information and to create an
account, please visit https://mega.co.nz/

POSIX (Linux/MacOS/BSD/...)
===========================

Install the following development packages, if available, or download and
compile their respective sources:

- GNU Readline (libreadline-dev, readline-devel)
- Crypto++ (libcrypto++-dev, cryptopp-devel)
- FreeImage (libfreeimage-dev, freeimage-devel)
- Berkeley DB C++ (libdb++-dev, db4-devel)
- cURL (libcurl-dev, curl-devel)

CAUTION: Verify that the installed libcurl uses c-ares for asynchronous name
resolution. If that is not the case, compile it from the original sources
with --enable-ares. Do NOT use --enable-threaded-resolver, which will cause
the engine to hang. Bear in mind that by not enabling asynchronous DNS
resolving at all, the engine is no longer nonblocking.

If you compiled any components yourself, it may be necessary to modify the
supplied Makefile to reflect their locations.

When done, build and run megaclient:

make
./megaclient

Ensure that your terminal supports UTF-8 if you want to see and manipulate
non-ASCII filenames.

Windows
=======

To build megaclient.exe under Windows, you'll need to following:

- A Windows-native C++ development environment (e.g. MinGW or Visual Studio)
- GNU Readline/Termcap (original sources or precompiled)
- Crypto++ (original sources or precompiled)
- Berkeley DB C++ (original sources or precompiled)
- FreeImage (original sources or precompiled)

(You do not need cURL, as megaclient's Win32 version relies on WinHTTP for
network access.)

First, compile the components that you have obtained as source code.

If needed, modify the supplied Makefile.win32 to reflect the locations of
the components that you installed, then use make / mingw32-make / nmake
to build megaclient.exe.

CAUTION: megaclient is a console application and expects to run in a UTF-8
terminal. Unfortunately, the Windows-native cmd.exe is not one of these,
so foreign characters in filenames will appear garbled. This issue does
not affect uploaded or downloaded files.

Further limitations of the Windows version:

- Local terminal echo is not turned off for password entry
- Upload progress is reported only upon chunk completion
- Thumbnail generation is unsupported for non-ASCII filenames
