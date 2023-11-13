[[_TOC_]]

# Ubuntu instructions

We officially support a handful of Linux flavors based on Debian and RedHat,
however, for development, we are focusing on Ubuntu LTS as a good reference platform.
Current LTS is 22.04 (jammy) and our developers use that for daily Linux development.

# Requirements

Important requirements for MEGAsync are GCC above version 5 and Qt above version 5.15. The
other third-party requirements will vary from distro to distro; we will aim for a
base-line here and rely on you for the necessary adjustments for your specific Linux
installation. If you need help, you can always reach out to us on GitHub.

## Tools

Some essential tools are needed to get the building process going. Here is the
example Debian APT install command:

```
$ sudo apt install git build-essential wget dh-autoreconf cdbs unzip libtool-bin pkg-config debhelper
```

The package `build-essential on Ubuntu, is responsible for installing g++, autoconf, automake and a couple other build essential tools. Please install these packages or their available equivalent in your distribution's package repository.

## APT Libraries

Here is the third-party dependency list for APT based distros (e.g. Ubuntu):
```
$ sudo apt install qttools5-dev-tools qtbase5-dev qt5-qmake libqt5x11extras5-dev libqt5dbus5 libqt5svg5-dev qtdeclarative5-dev qml-module-qtquick-dialogs qml-module-qtquick-controls qml-module-qtquick-controls2
$ sudo apt install libcrypto++-dev libraw-dev libc-ares-dev libssl-dev sqlite3 libsqlite3-dev zlib1g-dev \
    libavcodec-dev libavutil-dev libavformat-dev libswscale-dev mediainfo libfreeimage-dev \
    libreadline-dev libsodium-dev libuv1 libuv1-dev libudev-dev libzen-dev libx11-dev libx11-xcb-dev libgl-dev \
    libz-dev libicu-dev libmediainfo-dev
```
Optionally, if you wish to build the Gnome\Nautilus extension:

```
$ sudo apt-get install libnautilus-extension-dev
```

# Get the source

Open a Terminal and clone the MEGAsync repository:
```
$ mkdir ~/mega
$ cd ~/mega
$ git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```
The MEGA SDK is fetched recursively from https://github.com/meganz/sdk.git

# Build everything

The first step is to configure the MEGA SDK:
```
$ cd desktop/src/MEGASync/mega/
$ ./autogen.sh
$ ./configure
```
You don't have to build the SDK at this point via `make`, since it will be built by the `MEGAsync.pro project`.

Then build the Desktop app:
```
cd ../..
qmake MEGASync/MEGASync.pro
lrelease MEGASync/MEGASync.pro
make -j $(nproc)
```

# Development

For development, you can open `desktop/src/MEGASync/MEGASync.pro` in Qt Creator
IDE, which can be installed on Debian flavors like so:
```
$ sudo apt install qtcreator libclang-common-11-dev
```

When building via Qt Creator, note that the SDK is also being rebuilt, since it has its
own QMake project file in the SDK sub-project at `bindings/qt/sdk.pri` included by the
parent QMake.

You might have to generate the initial set of language files, so they are found by the
build system onwards. To do that, in Qt Creator, in the application menu, go to Tools ->
External -> Linguist and click on Release Translations action.

