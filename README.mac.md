# macOS instructions

For MEGA Desktop App development we are targeting macOS Big Sur, although we are providing
packages for other versions as well; please check the main README file for a definitive
list.

# Tools

Downloading the latest stable versions of these tools should generally be Ok.

## Xcode & CMake

Install latest XCode via App Store using your Apple account.

For MEGA Desktop development, we have a working headless build system via CMake
which we are using the build the installer packages, however, for day-to-day
development we are primarily using QMake as a team, due to having good Qt
tooling support and a working system for all the supported platforms. That being
said, we are using CMake right now to build the 3rdParty dependencies required
for both SDK and MEGA Desktop and then switching over to QMake + Qt Creator for
developer convenience.

Please download and copy CMake.app into /Applications (or custom location) using the macOS
dmg Installer from:
https://cmake.org/download/

It is useful to add CMake to the system path in the installer wizard. (for current user is
fine), so run the following command:
```
$ sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
```

## YASM (latest version from upstream)

Download and install YASM which is required to build ffmpeg dependency.
Follow these steps:
```
$ git clone https://github.com/yasm/yasm/
$ cd yasm
$ cmake .
$ cmake --build . 
$ sudo cmake --install
```

# Third-Party dependencies

## Qt SDK (5.12.11)

Install Qt Open Source and Qt Creator using the Qt Online Installer from:
https://www.qt.io/download-qt-installer

You will have to create an account, even if you only install the Community Editions.
Install Qt 5.12.11 for macOS; only macOS components are needed during installation.
A good installation path is `~/Qt`.

## VCPKG

Along with Qt, MEGA Desktop and the MEGA SDK require another dozen or more
3rdParty libraries to cover all the functionality exposed to our users. We are
using Microsoft's VCPKG C++ Library Manager for managing our dependencies and we
employ it automagically from our CMake scripts. You don't have to install it
manually.

A notable exception from this rule, is the PDFIUM library, which is used to
create thumbnails for PDF documents. This library is not available in VCPKG for
the moment, thus we are using a patched version from upstream Chromium Depot
Tools for the moment. Download our pre-built version from:
https://mega.nz/file/M1JCRCRa#Ne5sbVD2yZaCt9ijcCaKXs3m_ayfrw0ZovJMdERXRlU

Start decompressing the zip archive as you proceed to the next step.

# Get the source

Open macOS Terminal and clone the Desktop repository:
```
$ mkdir ~/mega/
$ cd ~/mega/
$ git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```

The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

This step, will check-out all the 3rdParty dependencies via VCPKG and then
proceed to build those, the SDK and MEGA Desktop in that order, via CMake.

```
$ cd ~/mega/desktop/contrib/cmake
$ cmake -DEXTRA_ARGS="-DCMAKE_PREFIX_PATH=~/Qt/5.12.11/clang_64" -DTARGET=MEGAsync -DTRIPLET=x64-osx-mega -P build_from_scratch.cmake
```

In a short while, but before the build stops because of missing PDFIUM library, you should
notice `~/mega/3rdparty_desktop/vcpkg` directory being created. You can start copying
the extracted pdfium subdirectory to the vcpkg directory.

In case the build fails anyway, just run the cmake command again, once pdfium is copied.

# Development using Qt Creator

Now you can open open `src/MEGASync/MEGASync.pro` to start editing and building. Set it up
as any other Qt QMake based project, using the Qt 5.12.11 kit you installed and set
matching target architecture.

When building using the QMake project, both the application and the SDK are
being rebuilt since the latter has its own QMake project files in the
sub-project at bindings/qt/sdk.pri. Whereas the 3rdParty libs remain the ones
being already built by CMake.

You might have to generate the initial set of language files so they are found
by the build system onwards. To do that, in Qt Creator, in the application menu,
go to Tools -> External -> Linguist and click on Release Translations action.
You can achieve the same, from the command-line:
```
$ cd ~/mega/desktop/src
$ ~/Qt/5.12.11/clang_64/bin/lrelease MEGASync/MEGASync.pro
```
