[[_TOC_]]

# Windows instructions

For MEGA Desktop App development we are targeting Windows 10, although we are providing
packages for other versions as well; please check the main README file for a definitive
list.

# Tools

Downloading the latest stable versions of these tools should generally be Ok.

## Visual Studio 2019

Install the Community Edition via the Visual Studio Installer. If you have a
Professional license, that works too. Along with the core editor package you
will need to install the extra Workload Component for `Desktop Development with C++`.

## CMake

For MEGA Desktop development, we have a working headless build system via CMake
which we are using the build the installer packages, however, for day-to-day
development we are primarily using QMake as a team, due to having good Qt
tooling support and a working system for all the supported platforms. That being
said, we are using CMake right now to build the 3rdParty dependencies required
for both SDK and MEGA Desktop and then switching over to QMake + Qt Creator for
developer convenience.

Please download and install using the Windows x64 Installer from:
https://cmake.org/download/

It is useful to add CMake to the system path in the installer wizard. (for
current user only is fine)

## Git

Download Git for Windows based on the MSYS MinGW environment from:
https://git-scm.com/downloads

During the installation wizard:
- Select "Checkout as-is, commit as-is" to avoid line ending conversions.
- Add the Git tools to the user environment PATH;

This being the simplest option, the alternative is to use it from git-bash only.
Keep in mind that CMake and Visual Studio Compiler tools need to be accessible
from inside your shell.

## Using the tools

We are using the new Windows Terminal with PowerShell and the vanilla prompt to
build from the command-line. Using PowerShell in the recent Windows Terminal with
no specific development environment loaded up, works best for our use case.
Since you already added CMake and Git to your user environment paths, they are
responsible for supplying all of the other tools necessary for building. Most
notably, the exact VC++ compiler and builder tool from Visual Studio. For more
complex Git operations or using authentication, you likely have to revert to
using the Git Bash version of the Git Tools package.

# Third-Party dependencies

## Qt 5 SDK

Install Qt Open Source and Qt Creator using the Qt Online Installer from:
https://www.qt.io/download-qt-installer

You will have to create an account, even if you only install the Community
Editions. Install Qt 5.12.11 for MSVC 2017 and Qt Creator + Debugger Tools. There is no
Qt 5.12.11 built with MSVC 2019 available. A good installation path is `C:\Qt\`.

## VCPKG

Along with Qt, MEGA Desktop and the MEGA SDK require another dozen or more
3rdParty libraries to cover all the functionality exposed to our users. We are
using Microsoft's VCPKG C++ Library Manager for managing our dependencies and we
employ it automagically from our CMake scripts. You don't have to install it
manually.

A notable exception from this rule, is the PDFIUM library, which is used to
create thumbnails for PDF documents. This library is not available in VCPKG for
the moment, thus we are using a patched version from upstream Chromium Depot
Tools. Download the patched pdfium source repository from:
https://mega.nz/file/g4AkzKzK#jaHHRwPyX4Ql9882A5wROz4j92TW3rzRunvvibDg37Y

Start decompressing the zip archive as you proceed to the next step.

# Get the source

Open Windows Terminal and clone the Desktop repository:
```
$ mkdir c:\mega\
$ cd c:\mega\
$ git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```

The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

This step, will check-out all the 3rdParty dependencies via VCPKG and then
proceed to build those, the SDK and MEGA Desktop in that order, via CMake.

```
$ cd c:\mega\desktop\contrib\cmake
$ cmake -DEXTRA_ARGS="-DCMAKE_PREFIX_PATH=C:\Qt\5.12.11\msvc2017_64" -DTARGET=MEGAsync -DTRIPLET=x64-windows-mega -P build_from_scratch.cmake
```

To change the target (to x86 for e.g.), choose another triplet name from:
`c:\mega\desktop\src\MEGASync\mega\contrib\cmake\vcpkg_extra_triplets\`
To change the Visual Studio version used, edit the target triplet file.

In a short while, but before the build stops because of missing PDFIUM library,
you should notice `c:\mega\3rdparty_desktop\vcpkg` directory being created.
You can start copying the extracted `pdfium` subdirectory to the `vcpkg`
directory like so:
`c:\mega\3rdparty_desktop\vcpkg\pdfium\pdfium\`

In case the build fails anyway, just run the cmake command again, once pdfium is
copied.

# Development using Qt Creator

For development with an IDE, we recommend Qt Creator, though Visual Studio
should work fine with the CMake generated solution file. Opening the CMakeLists
in both IDE's should be fine too, though we are not using this approach right
For development with an IDE, we recommend Qt Creator, though Visual Studio
should work fine with the CMake generated solution file. Opening the CMakeLists
in both IDE's should be fine too.

Open `desktop\src\MEGAsync\MEGAsync.pro` in Qt Creator. Set it up as any
other Qt QMake based project, using the Qt 5.12.11 MSVC 2017 kit you installed
and set matching target architecture. Using that kit is fine, even if you only
installed MSVC 2019.

Some recommended options:
- disable `Run in terminal`
- check `Add build library search path to PATH`
- in the `Build environment` section, set the library search path added
by the previous option to use the `\bin` directory instead of `\lib`, like
so:
`c:\mega\desktop\src\MEGASync\..\..\..\3rdParty_desktop\vcpkg\installed\x64-windows-mega\debug\bin`

This ensures that MEGASync will find the required dynamic libraries at run-time
when started from inside the IDE. Change the parent directory to release if
you're targeting that.

When building using the QMake project, both the application and the SDK are
being rebuilt since the latter has its own QMake project files in the
sub-project at `bindings/qt/sdk.pri`. Whereas the 3rdParty libs remain the ones
being already built by CMake.

You might have to generate the initial set of language files so they are found
by the build system onwards. To do that, in Qt Creator, in the application menu,
go to Tools -> External -> Linguist and click on Release Translations action.
You can achieve the same, from the command-line:
```
$ cd c:\mega\desktop\src
$ c:\Qt\5.12.11\msvc2017_x64\bin\lrelease.exe MEGASync/MEGASync.pro
```

