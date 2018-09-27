MEGA Sync Client
================

Easy automated syncing between your computers and your MEGA cloud drive.

This repository contains all the development history of the official sync client of MEGA:
https://mega.nz/sync

#### Supported Platforms

* Microsoft Windows operating systems
* OS X
* Linux

#### Get the source
```
git clone --recursive https://github.com/meganz/MEGAsync.git
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
https://mega.nz/#!YwEQzBCK!10kFLaIU8_yEL87Md_GowLdQLE1sxvM3yOngOrQy_Wk

5.- Uncompress that file into `src\MEGASync\mega\bindings\qt`

6.- Open the project `src/MEGA.pro` with QTCreator

7.- Select the folder `Release_x32` as the target for Release and Debug builds

8.- Build the project

9.- Copy or move the .dll files from the folder `src\MEGASync\mega\bindings\qt\3rdparty\bin` to the folder `Release_x32\MEGASync`

10.-  Enjoy!

It's recommended to go to `Project -> Run` in QTCreator and disable the option `Run in terminal`

# OS X compilation

##### Requirements:
* Xcode
* QT 5.6.2

##### Preparation:
1.- Install Xcode in your system

2.- Clone or download this repo

3.- Download the required third party libraries and configuration file (`config.h`) from this link:
https://mega.nz/#!m0AkyAAQ!dTZhq9IKzRrL5Kl5zCQkAk2X3iCuf2uEbSfdZ-OyLnU

4.- Uncompress that file and move the folder `3rdparty` into `src/MEGASync/mega/bindings/qt`/ and the file `config.h` into `src/MEGASync/mega/include/mega/`

5.- Run the script `installer_mac.sh` to build the project and generate the application bundle for MEGAsync. If you want to generate an Apple disk image (DMG file), add the flag `--create-dmg`. Build directory is `Release_x64`

6.- Enjoy!

## Linux compilation

Preparation:

First install dependencies. e.g., for a debian/ubuntu with QT >= 5.6 (e.g: Ubuntu 18.04):
```
sudo apt-get install libzen-dev libmediainfo-dev debhelper qtbase5-dev qt5-qmake qt4-linguist-tools libqt5dbus5 libqt5svg5-dev libcrypto++-dev libraw-dev libc-ares-dev libssl-dev libsqlite3-dev zlib1g-dev wget dh-autoreconf cdbs unzip libtool-bin pkg-config qt5-default qttools5-dev-tools libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libmediainfo-dev
```

for older debian/ubuntu based systems (e.g: Ubuntu 16.04):
```
sudo apt-get install build-essential autoconf automake m4 libtool libtool-bin qt4-qmake make libqt4-dev libcrypto++-dev libsqlite3-dev libc-ares-dev libcurl4-openssl-dev libssl-dev libraw-dev libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libmediainfo-dev
# Optional, if you wish to build nautilus extension:
sudo apt-get install libnautilus-extension-dev
```

Addition: for RPM based (e.g: Fedora): 
```
sudo dnf install libtool gcc-c++ c-ares-devel cryptopp-devel openssl-devel qt-devel sqlite-devel zlib-devel LibRaw-devel
```

Building:
```
cd src
./configure
qmake MEGA.pro
lrelease MEGASync/MEGASync.pro
make
```

Note: when compiling for Fedora/RHEL/CentOS and alike, `qmake` and `lrelease` might be missing for qt4. Use `qmake-qt4` and `lrelease-qt4` instead. Also, adding `-q` to `configure` is recommended in order to download and build cryptopp for RHEL and CentOS.

Known Issues
------------
For Solus, c-ares might not compile do to a CFLAG defined by gcc: -D_FORTIFY_SOURCE=2. This issue and its possible solution is described here https://github.com/c-ares/c-ares/issues/58 and http://ma.tc/ehuboqatec.md.
