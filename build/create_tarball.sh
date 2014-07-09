#!/bin/sh

# make sure the source tree is in "clean" state
cwd=$(pwd)
cd ../Source
make distclean 2> /dev/null || true
cd MEGASync
make distclean 2> /dev/null || true
cd sdk
rm -fr 3rdparty/*
cd sdk
make distclean 2> /dev/null || true
cd $cwd

# download cURL
export CURL_VERSION=7.32.0
export CURL_SOURCE_FILE=curl-$CURL_VERSION.tar.gz
export CURL_SOURCE_FOLDER=curl-$CURL_VERSION
export CURL_DOWNLOAD_URL=http://curl.haxx.se/download/$CURL_SOURCE_FILE
if [ ! -f $CURL_SOURCE_FILE ]; then
    wget -c $CURL_DOWNLOAD_URL || exit 1
fi

# download libsodium
export SODIUM=libsodium
export SODIUM_VERSION=0.5.0
export SODIUM_SOURCE_FILE=$SODIUM-$SODIUM_VERSION.tar.gz
export SODIUM_SOURCE_FOLDER=$SODIUM-$SODIUM_VERSION
export SODIUM_DOWNLOAD_URL=https://download.libsodium.org/libsodium/releases/$SODIUM_SOURCE_FILE
if [ ! -f $SODIUM_SOURCE_FILE ]; then
    wget -c $SODIUM_DOWNLOAD_URL || exit 1
fi

# get current version
MEGASYNC_VERSION=`grep -Po 'const QString MegaApplication::VERSION_STRING = QString::fromAscii\("\K[^"]*' ../Source/MEGASync/MegaApplication.cpp`
export MEGASYNC_NAME=megasync-$MEGASYNC_VERSION
rm -rf $MEGASYNC_NAME.tar.gz
rm -rf $MEGASYNC_NAME

echo "MEGASync version: $MEGASYNC_VERSION"

# delete previously generated files
rm -fr MEGASync/MEGASync/megasync_*.dsc
# fix version number in template files and copy to appropriate directories
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGASync/megasync.spec > MEGASync/MEGASync/megasync.spec
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGASync/megasync.dsc > MEGASync/MEGASync/megasync_$MEGASYNC_VERSION.dsc
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGASync/PKGBUILD > MEGASync/MEGASync/PKGBUILD

# create archive
mkdir $MEGASYNC_NAME
ln -s ../MEGASync/MEGASync/megasync.spec $MEGASYNC_NAME/megasync.spec
ln -s ../../Source/configure $MEGASYNC_NAME/configure
ln -s ../../Source/MEGA.pro $MEGASYNC_NAME/MEGA.pro
ln -s ../../Source/MEGASync $MEGASYNC_NAME/MEGASync
ln -s ../$CURL_SOURCE_FILE $MEGASYNC_NAME/$CURL_SOURCE_FILE
ln -s ../$SODIUM_SOURCE_FILE $MEGASYNC_NAME/$SODIUM_SOURCE_FILE
tar czfh $MEGASYNC_NAME.tar.gz --exclude $MEGASYNC_NAME/MEGASync/sdk/sqlite3.c --exclude Makefile --exclude '*.o' $MEGASYNC_NAME
rm -rf $MEGASYNC_NAME

# delete any previous archive
rm -fr MEGASync/MEGASync/megasync_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $MEGASYNC_NAME.tar.gz MEGASync/MEGASync/megasync_$MEGASYNC_VERSION.tar.gz


# make sure the source tree is in "clean" state
cd ../Source/MEGAShellExtNautilus/
make distclean 2> /dev/null || true
cd ../../build

# extension uses the same version number as MEGASync app
export EXT_VERSION=$MEGASYNC_VERSION
export EXT_NAME=nautilus-megasync-$EXT_VERSION
rm -rf $EXT_NAME.tar.gz
rm -rf $EXT_NAME

# delete previously generated files
rm -fr MEGASync/MEGAShellExtNautilus/nautilus-megasync_*.dsc

# fix version number in template files and copy to appropriate directories
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/nautilus-megasync.spec > MEGASync/MEGAShellExtNautilus/nautilus-megasync.spec
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/nautilus-megasync.dsc > MEGASync/MEGAShellExtNautilus/nautilus-megasync_$EXT_VERSION.dsc
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/PKGBUILD > MEGASync/MEGAShellExtNautilus/PKGBUILD

# create archive
mkdir $EXT_NAME
ln -s ../MEGASync/MEGAShellExtNautilus/nautilus-megasync.spec $EXT_NAME/nautilus-megasync.spec
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_client.c $EXT_NAME/mega_ext_client.c
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_client.h $EXT_NAME/mega_ext_client.h
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_module.c $EXT_NAME/mega_ext_module.c
ln -s ../../Source/MEGAShellExtNautilus/mega_notify_client.h $EXT_NAME/mega_notify_client.h
ln -s ../../Source/MEGAShellExtNautilus/mega_notify_client.c $EXT_NAME/mega_notify_client.c
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExt.c $EXT_NAME/MEGAShellExt.c
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExt.h $EXT_NAME/MEGAShellExt.h
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExtNautilus.pro $EXT_NAME/MEGAShellExtNautilus.pro
ln -s ../../Source/MEGAShellExtNautilus/data $EXT_NAME/data
export GZIP=-9
tar czfh $EXT_NAME.tar.gz --exclude Makefile --exclude '*.o' $EXT_NAME
rm -rf $EXT_NAME

# delete any previous archive
rm -fr MEGASync/MEGAShellExtNautilus/nautilus-megasync_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $EXT_NAME.tar.gz MEGASync/MEGAShellExtNautilus/nautilus-megasync_$EXT_VERSION.tar.gz
