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

## NASM

Download and install NASM which is required to build ffmpeg dependency.
Follow these steps:
```
# Get autoconf and automake, build them and install them in a local folder
$ curl -O -L https://ftpmirror.gnu.org/gnu/autoconf/autoconf-2.71.tar.xz
$ curl -O -L https://ftpmirror.gnu.org/gnu/automake/automake-1.16.5.tar.xz
$ tar xzf autoconf-2.71.tar.xz
$ tar xzf automake-1.16.5.tar.xz
$ mkdir bin_dest
$ export PATH=$PATH:$PWD/bin_dest/bin
$ cd autoconf-2.71
$ ./configure --prefix=$PWD/../bin_dest
$ make install
$ cd ..
$ cd automake-1.16.5
$ ./configure --prefix=$PWD/../bin_dest
$ make install
$ autoconf --version
$ automake --version

# Get, build and install nasm in the system
$ curl -O -L https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/nasm-2.16.01.tar.xz
$ tar xzf nasm-2.16.01.tar.xz
$ cd nasm-2.16.01
$ ./configure
$ make
$ sudo make install
$ nasm --version
# Clean up
$ cd ..
$ rm -rf autoconf*
$ rm -rf automake*
$ rm -r nasm*
$ rm -r bin_dest
```

# Third-Party dependencies

## Qt SDK (5.12.12)

Install Qt Open Source and Qt Creator using the Qt Online Installer from:
https://www.qt.io/download-qt-installer

You will have to create an account, even if you only install the Community Editions.
Install Qt 5.12.12 for macOS; only macOS components are needed during installation.
A good installation path is `~/Qt`.

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
$ cmake -DEXTRA_ARGS="-DCMAKE_PREFIX_PATH=~/Qt/5.12.12/clang_64" -DTARGET=MEGAsync -DTRIPLET=x64-osx-mega -P build_from_scratch.cmake
```

# Development using Qt Creator

Now you can open open `src/MEGASync/MEGASync.pro` to start editing and building. Set it up
as any other Qt QMake based project, using the Qt 5.12.12 kit you installed and set
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
$ ~/Qt/5.12.12/clang_64/bin/lrelease MEGASync/MEGASync.pro
```
