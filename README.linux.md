[[_TOC_]]

# Linux instructions

For MEGA Desktop App development we are targeting Ubuntu LTS, although we are providing
packages for other Linux flavors as well; please check the main README file for a
definitive list.

MEGA Desktop App may run on any flavor of Linux as long as the Desktop Manager and Window
Environment is supported by the Qt SDK and the required third-party dependencies are
available in their package management repositories. The officially supported distros
guarantee that availability and MEGAsync compatibility with their default Window Manager
and Desktop environment. In this document we aim to provide instructions for Debian (APT
packaging) and RedHat (RPM packaging).

# Requirements

Important requirements for MEGAsync are GCC above version 5 and Qt above version 5.10. The
other third-party requirements will vary from distro to distro; we will aim for a
base-line here and rely on you for the necessary adjustments for your specific Linux
installation. If you need help, you can always reach out to us on GitHub.

## Tools

Some essential tools are needed to get the building process going. Here is the
example Debian APT install command:

```
$ sudo apt install git build-essential wget dh-autoreconf cdbs unzip libtool-bin pkg-config debhelper
```
And here is the example for Fedora:
```
$ sudo dnf install @"C Development Tools and Libraries" git wget unzip
```
The package `build-essential` on Debian and the `C Development Tools and Libraries` on
Fedora, are responsible for installing g++, autoconf, automake and some other build
essential tools. Please install these packages, or their available equivalent, in your
distribution's package repository.

The biggest differentiating factor in getting MEGAsync to work on your distro is, whether
you can install (or build) all the library dependencies we need.

## APT Libraries

Here is the third-party dependency list for APT based distros (e.g. Ubuntu):
```
$ sudo apt install qt5-default qttools5-dev-tools qtbase5-dev qt5-qmake libqt5x11extras5-dev libqt5dbus5 libqt5svg5-dev libcrypto++-dev libraw-dev libc-ares-dev libssl-dev libsqlite3-dev zlib1g-dev libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libmediainfo-dev libfreeimage-dev libreadline-dev libsodium-dev libuv1-dev libudev-dev libzen-dev
```

## RPM Libraries

Here is the dependency list for RPM based distros (e.g. Fedora):
```
$ sudo dnf install qt5-qtbase-devel qt5-qttools-devel qt5-qtsvg-devel qt5-qtx11extras-devel c-ares-devel cryptopp-devel openssl-devel sqlite-devel zlib-devel LibRaw-devel libudev-devel libzen-devel libmediainfo-devel
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

This steps, will check-out all the 3rdParty dependencies and then
proceed to build those, the SDK and MEGA Desktop:
```
$ cd ~/mega/desktop/src/
$ ./configure -g -q -i
$ qmake MEGASync/MEGASync.pro
$ lrelease MEGASync/MEGASync.pro
$ make
```
Note: when compiling for RPM based distros, `qmake` and `lrelease` programs might be missing.
Use `qmake-qt5` and `lrelease-qt5` instead.

# Development

For development, you can open `desktop/src/MEGASync/MEGASync.pro` in Qt Creator
IDE, which can be installed on Debian flavors like so:
```
$ sudo apt install qtcreator libclang-common-8-dev
```
or on Fedora running:
```
$ sudo dnf install qtcreator
```

When building via Qt Creator, note that the SDK is also being rebuilt, since it has its
own QMake project file in the SDK sub-project at `bindings/qt/sdk.pri` included by the
parent QMake.

You might have to generate the initial set of language files, so they are found by the
build system onwards. To do that, in Qt Creator, in the application menu, go to Tools ->
External -> Linguist and click on Release Translations action.

