MEGA Sync Client: Easy automated syncing between your computers and your MEGA cloud drive
=========================================================================================

### Supported Platforms

* Microsoft Windows operating systems
* Linux
* OS X

## Windows compilation

Requirements:
* Visual Studio 2010
* QT 4.0
* Qt Creator

### Linux compilation

Preparation:
```
sudo apt-get install build-essential autoconf automake m4 libtool qt4-qmake make libqt4-dev libcrypto++-dev libsqlite3-dev libfreeimage-dev libc-ares-dev
sudo apt-get install libnautilus-extension-dev
```

Get the source:
```
git clone https://github.com/meganz/desktop.git
git submodule update --init --recursive
```

Building:
```
cd Source
./configure
qmake MEGA.pro
lrelease MEGASync/MEGASync.pro
make
```
