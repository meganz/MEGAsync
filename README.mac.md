[[_TOC_]]

# macOS instructions

For MEGA Desktop App development we are targeting macOS Sonoma, although we are providing
packages for other versions as well; please check the main README file for a definitive
list.

# Tools

Downloading the latest stable versions of these tools should generally be Ok.

## Xcode & CMake

Install latest XCode via App Store using your Apple account.

We use CMake as our exclusive build system.

Please download and copy CMake.app into /Applications (or custom location) using the macOS
dmg Installer from:
https://cmake.org/download/

It is useful to add CMake to the system path in the installer wizard. (for current user is
fine), so run the following command:
```
$ sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
```

## System tools: autoconf, autoconf-archive, automake, NASM and pkg-config
In a working folder of your preference, get autoconf, autoconf-archive, and automake. Build and install them in the system.
```
curl -O -L https://ftpmirror.gnu.org/gnu/autoconf/autoconf-2.71.tar.xz
curl -O -L https://ftpmirror.gnu.org/gnu/automake/automake-1.16.5.tar.xz
curl -O -L https://ftpmirror.gnu.org/gnu/autoconf-archive/autoconf-archive-2023.02.20.tar.xz
tar xzf autoconf-2.71.tar.xz
tar xzf automake-1.16.5.tar.xz
tar xzf autoconf-archive-2023.02.20.tar.xz
cd autoconf-2.71
./configure
sudo make install
cd ..
cd automake-1.16.5
./configure
sudo make install
cd ..
cd autoconf-archive-2023.02.20
./configure
sudo make install
cd ..
# Check that both are working
autoconf --version
automake --version
```

Then download, build and install NASM and pkg-config:
```
curl -O -L https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/nasm-2.16.01.tar.xz
curl -O -L https://pkgconfig.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
tar xzf nasm-2.16.01.tar.xz
tar xzf pkg-config-0.29.2.tar.gz
cd nasm-2.16.01
./configure
$make
sudo make install
cd ..
cd pkg-config-0.29.2
./configure --with-internal-glib
make
sudo make install
cd ..
# Check that both are working
nasm --version
pkg-config --version
```

Clean-up:
```
rm -rf autoconf*
rm -rf automake*
rm -rf nasm*
rm -rf pkg-config*
```

# Third-Party dependencies

## Qt 5 SDK

You can install Qt Creator using the Qt Online Installer from:
https://www.qt.io/download-qt-installer

You will have to create an account, even if you only install the Community Editions.

We are currently using Qt 5.15 (LTS). The latest pre-built version provided by the Qt company is 5.15.2,
and is available from the Qt Maintenance Tool (in the "Archive" category).

You can also build a more recent version of Qt using:
```
cd contrib/build_qt/macOS
./build-qt.sh
```
Qt will be installed in `~/Qt-build/`.

The scripts expects to find `python` (version 3), `perl` and `jom` in the `PATH` environment variable.

## VCPKG

Along with Qt, MEGA Desktop and the MEGA SDK require another dozen or more
third party libraries to cover all the functionality exposed to our users. We are
using Microsoft's VCPKG C++ Library Manager for managing our dependencies.

You need to clone the VCPKG git repo (you can of course choose the local directory, `c:\mega\` is given as an example):
```
mkdir ~/mega/
cd ~/mega/
git clone https://github.com/microsoft/vcpkg
```

# Get the source

Open macOS Terminal and clone the Desktop repository:
```
cd ~/mega/
git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```

The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

## Run CMake
Run CMake to configure the project. We will build in a separate directory, outside of the source tree.
```
cd ~/mega/
cmake -DCMAKE_PREFIX_PATH='~/Qt-build/5.15.16/5.15.16/arm64' -DVCPKG_ROOT='~/mega/vcpkg' -S '~/mega/desktop' -B '~/mega/build-arm64'
```
Adapt the parameters to suit your needs:
```
cmake -DCMAKE_PREFIX_PATH='<Qt path>' -DVCPKG_ROOT='<VCPKG root path>' -S '<desktop app repository path>' -B '<build directory>'
```

The dependencies will be built at this stage from VCPKG.

The instructions above are to build for an arm64 target. If you want to build for x86_64, use:
```
cd ~/mega/
cmake -DCMAKE_OSX_ARCHITECTURES:UNINITIALIZED=x86_64 -DCMAKE_PREFIX_PATH='~/Qt-build/5.15.16/5.15.16/x86_64'  -DVCPKG_ROOT='~/mega/vcpkg' -S '~/mega/desktop' -B '~/mega/build-x64'
```

## Build the Desktop App
To build the Desktop App, use target `MEGAsync`.
You can build in `Debug` or `Release` configuration.
Example:
```
cmake --build '~/mega/build-arm64' --config Debug --target MEGAsync
```

# Development using Qt Creator
Open the `CMakeLists.txt` located at the root of the repository.

## Configure the project
Select the configuration that you want for the project, and set their build path (you can keep the defaults).

You will need to add the path to VCPKG to the CMake configuration. For each configuration, add a new `VCPKG_ROOT` key (in the "Initial Configuration" tab), and set the value to the VCPKG root directory (for instance `~/mega/vcpkg`).

Then click the "Re-configure with Initial Parameters" button. Wait for this process to finish (you can follow the progress in the "General messages" output).
You're good to go!