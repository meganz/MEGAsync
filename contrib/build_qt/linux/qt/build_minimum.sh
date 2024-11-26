#!/bin/sh
set -e

jflag=
jval=1
rebuild=0
download_only=0

while getopts 'j:df' OPTION
do
  case $OPTION in
  j)
      jflag=1
      jval="$OPTARG"
      ;;
  f)
      fake=1
      ;;
  d)
      download_only=1
      ;;
  ?)
      printf "Usage: %s: [-j concurrency_level] (hint: your cores + 20%%) [-B] [-d]\n" $(basename $0) >&2
      exit 2
      ;;
  esac
done
shift $(($OPTIND - 1))

if [ "$jflag" ]
then
  if [ "$jval" ]
  then
    printf "Option -j specified (%d)\n" $jval
  fi
fi

cd `dirname $0`
ENV_ROOT=`pwd`
. ./env.source

echo "#### Qt build ####"

ver=5.15.16
mainver=`echo $ver | awk -F"." '{print $1"."$2}'`
SRC_ARCHIVE="qt-everywhere-opensource-src-$ver.tar.xz"

# Download Qt package
wget -c "https://download.qt.io/official_releases/qt/$mainver/$ver/single/$SRC_ARCHIVE" -O "$SRC_ARCHIVE"

# Get patches
mkdir -p patches
for p in \
CVE-2023-32763-qtbase-5.15.diff \
CVE-2023-34410-qtbase-5.15.diff \
CVE-2023-37369-qtbase-5.15.diff \
CVE-2023-38197-qtbase-5.15.diff \
CVE-2023-43114-5.15.patch \
0001-CVE-2023-51714-qtbase-5.15.diff \
0002-CVE-2023-51714-qtbase-5.15.diff \
CVE-2024-25580-qtbase-5.15.diff \
CVE-2024-36048-qtnetworkauth-5.15.diff
do
  wget -c "https://download.qt.io/official_releases/qt/$mainver/$p" -O "patches/$p"
done

# Verify checksums
[ $download_only -eq 1 ] && exit 0
echo "CHECKSUM ... "
if ! echo fdd3a4f197d2c800ee0085c721f4bef60951cbda9e9c46e525d1412f74264ed7 "$SRC_ARCHIVE" | sha256sum -c - \
|| ! echo e8acecaea4d0e78408fc7049c44c7fb44804d459e7c7f700e08fe0b7d4052256 patches/CVE-2023-32763-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 01917eae0587b72f1969c303289a26bc1a148fea2eef7a64e05e6e86c1dcc178 patches/CVE-2023-34410-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 279c520ec96994d2b684ddd47a4672a6fdfc7ac49a9e0bdb719db1e058d9e5c0 patches/CVE-2023-37369-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 382c10ec8f42e2a34ac645dc4f57cd6b717abe6a3807b7d5d9312938f91ce3dc patches/CVE-2023-38197-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 14cc26aa465ec9a5cac6f6b4c91b2f802b12a8134a6ab897a45449c418ca98c1 patches/CVE-2023-43114-5.15.patch | sha256sum -c - \
|| ! echo 2129058a5e24d98ee80a776c49a58c2671e06c338dffa7fc0154e82eef96c9d4 patches/0001-CVE-2023-51714-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 99d5d32527e767d6ab081ee090d92e0b11f27702619a4af8966b711db4f23e42 patches/0002-CVE-2023-51714-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 7cc9bf74f696de8ec5386bb80ce7a2fed5aa3870ac0e2c7db4628621c5c1a731 patches/CVE-2024-25580-qtbase-5.15.diff | sha256sum -c - \
|| ! echo e5d385d636b5241b59ac16c4a75359e21e510506b26839a4e2033891245f33f9 patches/CVE-2024-36048-qtnetworkauth-5.15.diff | sha256sum -c -

then
exit 1
fi

# Extract sources
echo "EXTRACT SOURCES ... "
[ -d src ] && rm -r src;
mkdir -p src && tar xvf "$SRC_ARCHIVE" -C src --strip-components=1

# Apply patches
echo "APPLY PATCHES ... "
for p in \
CVE-2023-32763-qtbase-5.15.diff \
CVE-2023-34410-qtbase-5.15.diff \
CVE-2023-37369-qtbase-5.15.diff \
CVE-2023-38197-qtbase-5.15.diff \
CVE-2023-43114-5.15.patch \
0001-CVE-2023-51714-qtbase-5.15.diff \
0002-CVE-2023-51714-qtbase-5.15.diff \
CVE-2024-25580-qtbase-5.15.diff 
do
  echo "Applying patch: patches/$p"
  if ! patch -f --verbose -p1 -d src/qtbase < patches/$p ; then
    exit 1
  fi
done

for p in \
CVE-2024-36048-qtnetworkauth-5.15.diff
do
  echo "Applying patch: patches/$p"
  if ! patch -f --verbose -p1 -d src/qtnetworkauth < patches/$p ; then
    exit 1
  fi
done

# Build
echo "BUILD ... "
mkdir -p build && cd build
[ -f config.cache ] && rm config.cache; 

../src/configure --prefix=$TARGET_DIR/opt/mega \
  -opensource -confirm-license \
  -nomake examples -nomake tests \
  -skip qtwebview -skip qtwebengine -skip qtwebchannel -skip qtconnectivity -skip qt3d \
  -skip qtlocation -skip qtvirtualkeyboard \
  -ssl -fontconfig 2>&1 | tee salconfigure.txt

make -j $jval
make install
