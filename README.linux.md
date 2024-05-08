[[_TOC_]]

# Ubuntu instructions

We officially support a handful of Linux flavors based on Debian and RedHat,
however, for development, we are focusing on Ubuntu LTS as a good reference platform.
Current LTS is 22.04 (jammy) and our developers use that for daily Linux development.

# Requirements

Important requirements for MEGAsync are GCC above version 5 and Qt above version 5.15. The
other third-party requirements are handled through VCPKG.

## System build dependencies
In order to build the desktop app, you will need to install the following packages:
```
sudo apt install \
    build-essential \
    git \
    cmake \
    wget \
    autoconf-archive \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    nasm
```
and
```
sudo apt install \
    libxcb-cursor0 \
    qtbase5-dev \
    qttools5-dev  \
    libqt5x11extras5-dev  \
    libqt5svg5-dev    \
    qtdeclarative5-dev \
    qml-module-qtquick-dialogs \
    qml-module-qtquick-controls2
```
## VCPKG
Along with Qt, MEGA Desktop app and the MEGA SDK require another dozen or more
third party libraries to cover all the functionality exposed to our users. We are
using Microsoft's VCPKG C++ Library Manager for managing our dependencies.

You need to clone the VCPKG git repo (you can of course choose the local directory, `~/mega` is given as an example):
```
mkdir ~/mega
cd ~/mega
git clone https://github.com/microsoft/vcpkg
```

# Get the source
Open a Terminal and clone the MEGA Desktop app repository:

```
cd ~/mega
git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```
The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

## Run CMake
Run CMake to configure the project. We will build in a separate directory, outside of the source tree.
```
cd ~/mega
cmake -DVCPKG_ROOT='~/mega/vcpkg' -S '~/mega/desktop' -B '~/mega/build_dir' -DCMAKE_BUILD_TYPE=Debug
```
Adapt the parameters to suit your needs:
```
cmake -DVCPKG_ROOT='<VCPKG root path>' -S '<desktop app repository path>' -B '<build directory>' -DCMAKE_BUILD_TYPE=<Debug or Release>
```

The dependencies will be built at this stage from VCPKG.

## Build the Desktop App
To build the Desktop App, use target `MEGAsync`.
Example:
```
cmake --build '~/mega/build_dir' --target MEGAsync
```

The built executable will be in `~/mega/build_dir/src/MEGASync/Debug/`


# Development

For development, you can use the Qt Creator IDE, which can be installed on Debian flavors like so:
```
sudo apt install qtcreator
```
 Note: the Qt Online installer might provide a more recent version of Qt Creator than your distribution.

Open the `CMakeLists.txt` located at the root of the repository.

## Configure the project
Select the configuration that you want for the project, and set their build path (you can keep the defaults).

You will need to add the path to VCPKG to the CMake configuration. For each configuration, add a new `VCPKG_ROOT` key (in the "Initial Configuration" tab), and set the value to the VCPKG root directory (for instance `~/mega/vcpkg`).

Then click the "Re-configure with Initial Parameters" button. Wait for this process to finish (you can follow the progress in the "General messages" output).
You're good to go!