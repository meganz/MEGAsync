MEGA Sync Client
================

Easy automated syncing between your computers and your MEGA cloud drive.

This repository contains all the development history of the official sync client of MEGA:
https://mega.nz/#sync

#### Supported Platforms

* Microsoft Windows operating systems
* OS X
* Linux

#### Get the source
```
git clone https://github.com/meganz/desktop.git
git submodule update --init --recursive
```

# Windows compilation

##### Requirements:
* Visual Studio 2010
* QT 4.8
* Qt Creator

##### Preparation:
1.- Prepare QTCreator to use the Visual Studio 2010 toolchain and QT 4.8 - 32 bits

2.- Clone or download this repo

3.- Create a folder `Release_x32` inside the root of the code that you have just downloaded

4.- Download the required third party libraries from this link:
https://mega.nz/#!YkdC1QDB!V45YubgxVQq9aGF3oKLeCFYWcNdjVn3CzAUy-1ch-ps

5.- Uncompress that file into `src\MEGASync\mega\bindings\qt`

6.- Open the project `src/MEGA.pro` with QTCreator

7.- Select the folder `Release_x32` as the target for Release and Debug builds

8.- Build the project

9.- Copy or move the .dll files from the folder `src\MEGASync\mega\bindings\qt\3rdparty\bin` to the folder `Release_x32\MEGASync`

10.-  Enjoy!

It's recommended to to `Project -> Run` in QTCreator and disable the option `Run in terminal`

## Linux compilation

Preparation:
```
sudo apt-get install build-essential autoconf automake m4 libtool qt4-qmake make libqt4-dev libcrypto++-dev libsqlite3-dev libc-ares-dev
sudo apt-get install libnautilus-extension-dev
```

Building:
```
cd src
./configure
qmake MEGA.pro
lrelease MEGASync/MEGASync.pro
make
```

## End user installation instructions

* [Linux](INSTALL.md)
