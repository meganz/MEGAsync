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
0001-CVE-2023-51714-qtbase-5.15.diff \
0002-CVE-2023-51714-qtbase-5.15.diff \
CVE-2024-25580-qtbase-5.15.diff \
CVE-2024-36048-qtnetworkauth-5.15.diff \
CVE-2024-39936-qtbase-5.15.patch
do
  wget -c "https://download.qt.io/official_releases/qt/$mainver/$p" -O "patches/$p"
done

# Verify checksums
[ $download_only -eq 1 ] && exit 0
echo "CHECKSUM ... "
if ! echo efa99827027782974356aceff8a52bd3d2a8a93a54dd0db4cca41b5e35f1041c "$SRC_ARCHIVE" | sha256sum -c - \
|| ! echo 2129058a5e24d98ee80a776c49a58c2671e06c338dffa7fc0154e82eef96c9d4 patches/0001-CVE-2023-51714-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 99d5d32527e767d6ab081ee090d92e0b11f27702619a4af8966b711db4f23e42 patches/0002-CVE-2023-51714-qtbase-5.15.diff | sha256sum -c - \
|| ! echo 7cc9bf74f696de8ec5386bb80ce7a2fed5aa3870ac0e2c7db4628621c5c1a731 patches/CVE-2024-25580-qtbase-5.15.diff | sha256sum -c - \
|| ! echo e5d385d636b5241b59ac16c4a75359e21e510506b26839a4e2033891245f33f9 patches/CVE-2024-36048-qtnetworkauth-5.15.diff | sha256sum -c - \
|| ! echo 2cc23afba9d7e48f8faf8664b4c0324a9ac31a4191da3f18bd0accac5c7704de patches/CVE-2024-39936-qtbase-5.15.patch | sha256sum -c -


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
0001-CVE-2023-51714-qtbase-5.15.diff \
0002-CVE-2023-51714-qtbase-5.15.diff \
CVE-2024-25580-qtbase-5.15.diff \
CVE-2024-39936-qtbase-5.15.patch
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
