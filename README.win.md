[[_TOC_]]

# Windows instructions

For MEGA Desktop App development we are targeting Windows 10 and 11, although we are providing
packages for other versions as well; please check the main README file for a definitive
list.

# Tools

Downloading the latest stable versions of these tools should generally be OK.

## Visual Studio 2019

Install the Community Edition via the Visual Studio Installer. If you have a
Professional license, that works too. Along with the core editor package you
will need to install the extra Workload Component for `Desktop Development with C++`.

## CMake

We use CMake as our exclusive build system.

Please download and install using the Windows x64 Installer from:
https://cmake.org/download/

It is useful to add CMake to the system path in the installer wizard (for
current user only is fine).

Note that you may already have installed CMake through the `Qt Maintenance Tool` or `Visual Studio Installer`.

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

You can install Qt Creator using the Qt Online Installer from:
https://www.qt.io/download-qt-installer

You will have to create an account, even if you only install the Community
Editions.

We are currently using Qt 5.15 (LTS). The latest pre-built version provided by the Qt company is 5.15.2,
and is available from the Qt Maintenance Tool (in the "Archive" category).

You can also build a more recent version of Qt using:

```
cd contrib\build_qt\windows
.\build-qt.cmd
```
The scripts expects to find `python` (version 3), `perl` and `jom` in the `PATH` environment variable.

## VCPKG

Along with Qt, MEGA Desktop app and the MEGA SDK require another dozen or more
third party libraries to cover all the functionality exposed to our users. We are
using Microsoft's VCPKG C++ Library Manager for managing our dependencies.

You need to clone the VCPKG git repo (you can of course choose the local directory, `c:\mega\` is given as an example):
```
mkdir c:\mega\
cd c:\mega\
git clone https://github.com/microsoft/vcpkg
```

# Get the source

Open Windows Terminal and clone the Desktop repository:
```
cd c:\mega\
git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```

The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

## Run CMake
Run CMake to configure the project. We will build in a separate directory, outside of the source tree.
```
cd c:\mega\
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_PREFIX_PATH='C:\Qt\5.15.13\x64' -DVCPKG_ROOT='c:\mega\vcpkg' -S 'c:\mega\desktop' -B 'c:\mega\build-x64'
```
Adapt with the paths from your system:
```
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_PREFIX_PATH='<Qt path>' -DVCPKG_ROOT='<VCPKG root path>' -S '<desktop app repository path>' -B '<build directory>'
```

The dependencies will be built at this stage from VCPKG.

The instructions above are to build for a 64 bit target. If you want to build for 32 bit, use:
```
cd c:\mega\
cmake -DCMAKE_GENERATOR_PLATFORM=Win32 -DCMAKE_PREFIX_PATH='C:\Qt\5.15.13\x86' -DVCPKG_ROOT='c:\mega\vcpkg' -S 'c:\mega\desktop' -B 'c:\mega\build-x86'
```

## Build the Desktop App
To build the Desktop App, use target `MEGAsync`.
You can build in `Debug` or `Release` configuration.
Example:
```
cmake --build 'c:\mega\build-x64' --config Debug --target MEGAsync
```

The built executable will be in `.\src\MEGASync\Debug\`

# Development using Qt Creator
For development with an IDE, we recommend Qt Creator, though Visual Studio
should work fine. 

## Configure the project
Open the `CMakeLists.txt` located at the root of the repository.


Select the configuration that you want for the project, and set their build path (you can keep the defaults).

You will need to add the path to VCPKG to the CMake configuration. For each configuration, add a new `VCPKG_ROOT` key (in the "Initial Configuration" tab), and set the value to the VCPKG root directory (for instance `c:\mega\vcpkg`).

Then click the "Re-configure with Initial Parameters" button. Wait for this process to finish (you can follow the progress in the "General messages" output).
You're good to go!