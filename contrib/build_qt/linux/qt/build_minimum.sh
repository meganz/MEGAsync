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

ver=5.15.17
mainver=`echo $ver | awk -F"." '{print $1"."$2}'`
SRC_ARCHIVE="qt-everywhere-opensource-src-$ver.tar.xz"

# Download Qt package
wget -c "https://download.qt.io/official_releases/qt/$mainver/$ver/single/$SRC_ARCHIVE" -O "$SRC_ARCHIVE"

# Get patches
mkdir -p patches
for p in \
CVE-2024-39936-qtbase-5.15.patch \
CVE-2025-30348-qtbase-5.15.diff \
CVE-2025-4211-qtbase-5.15.diff \
CVE-2025-5455-qtbase-5.15.patch
do
  wget -c "https://download.qt.io/official_releases/qt/$mainver/$p" -O "patches/$p"
done

# Verify checksums
[ $download_only -eq 1 ] && exit 0
echo "CHECKSUM ... "
if ! echo 85eb566333d6ba59be3a97c9445a6e52f2af1b52fc3c54b8a2e7f9ea040a7de4 "$SRC_ARCHIVE" | sha256sum -c - \
|| ! echo 2cc23afba9d7e48f8faf8664b4c0324a9ac31a4191da3f18bd0accac5c7704de patches/CVE-2024-39936-qtbase-5.15.patch | sha256sum -c - \
|| ! echo 7bc92fb0423f25195fcc59a851570a2f944cfeecbd843540f0e80f09b6b0e822 patches/CVE-2025-4211-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 967fe137ee358f60ac3338f658624ae2663ec77552c38bcbd94c6f2eff107506 patches/CVE-2025-5455-qtbase-5.15.patch | sha256sum -c - \
|| ! echo fcd011754040d961fec1b48fe9828b2c8d501f2d9c30f0f475487a590de6d3c8 patches/CVE-2025-30348-qtbase-5.15.diff | sha256sum -c -
# || ! echo 967fe137ee358f60ac3338f658624ae2663ec77552c38bcbd94c6f2eff107506 CVE-2025-23050-qtconnectivity-5.15.diff | sha256sum -c -

then
exit 1
fi

# Extract sources
echo "EXTRACT SOURCES ... "
[ -d src ] && rm -r src;
mkdir -p src && tar xf "$SRC_ARCHIVE" -C src --strip-components=1

# Apply patches
echo "APPLY PATCHES ... "
for p in \
CVE-2024-39936-qtbase-5.15.patch \
CVE-2025-30348-qtbase-5.15.diff \
CVE-2025-4211-qtbase-5.15.diff \
CVE-2025-5455-qtbase-5.15.patch
do
  echo "Applying patch: patches/$p"
  if ! patch -f --verbose -p1 -d src/qtbase < patches/$p ; then
    exit 1
  fi
done

# for p in \
# CVE-2025-23050-qtconnectivity-5.15.diff
# do
#   echo "Applying patch: patches/$p"
#   if ! patch -f --verbose -p1 -d src/qtconnectivity < patches/$p ; then
#     exit 1
#   fi
# done

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
