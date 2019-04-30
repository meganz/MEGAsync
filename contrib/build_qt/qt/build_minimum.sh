#!/bin/sh
set -e
#set -u

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
##this is our working directory
#cd $BUILD_DIR

#download and extract package
if [ ! -f qt-everywhere-opensource-src-5.9.7.tar.xz ]; then
wget http://mirrors.ukfast.co.uk/sites/qt.io/archive/qt/5.9/5.9.7/single/qt-everywhere-opensource-src-5.9.7.tar.xz
fi

[ $download_only -eq 1 ] && exit 0
#TODO: include checksum

tar xvf qt-everywhere-opensource-src-5.9.7.tar.xz
cd qt-everywhere-opensource-src-5.9.7

[ -f config.cache ] && rm config.cache; 

./configure --prefix=$TARGET_DIR/opt/mega \
-opensource -ssl -confirm-license -nomake examples -nomake tests -nomake tools \
-skip xmlpatterns -skip qtwebview -skip qtwebengine -skip qtwebchannel \
 --xkbcommon-x11=qt -xkbcommon-evdev -fontconfig \
 -I /temporal/tools/openssl/1.0.2r/include 2>&1 | tee salconfigure.txt

make -j $jval
make install

