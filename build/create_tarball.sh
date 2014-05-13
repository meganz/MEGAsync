#!/bin/sh

# make sure the source tree is in "clean" state
cd ../Source
make distclean || true
cd ../build

# download cURL
export CURL_VERSION=7.32.0
export CURL_SOURCE_FILE=curl-$CURL_VERSION.tar.gz
export CURL_SOURCE_FOLDER=curl-$CURL_VERSION
export CURL_CONFIGURED=$CURL/$CURL_SOURCE_FOLDER/curl-config
export CURL_DOWNLOAD_URL=http://curl.haxx.se/download/$CURL_SOURCE_FILE
if [ ! -f $CURL_SOURCE_FILE ]; then
    wget -c $CURL_DOWNLOAD_URL || exit 1
fi

# prepare archieve
export MEGASYNC_VERSION=1.0
export MEGASYNC_NAME=megasync-$MEGASYNC_VERSION
rm -rf $MEGASYNC_NAME.tar.gz
rm -rf $MEGASYNC_NAME
mkdir $MEGASYNC_NAME
ln -s ../MEGASync/MEGASync/megasync.spec $MEGASYNC_NAME/megasync.spec
ln -s ../../Source/configure $MEGASYNC_NAME/configure
ln -s ../../Source/MEGA.pro $MEGASYNC_NAME/MEGA.pro
ln -s ../../Source/MEGASync $MEGASYNC_NAME/MEGASync
ln -s ../$CURL_SOURCE_FILE $MEGASYNC_NAME/$CURL_SOURCE_FILE
tar cvzfh $MEGASYNC_NAME.tar.gz --exclude $MEGASYNC_NAME/MEGASync/sdk/sqlite3.c $MEGASYNC_NAME
rm -rf $MEGASYNC_NAME

# transform arch name, to satisfy Debian requirements
mv $MEGASYNC_NAME.tar.gz MEGASync/MEGASync/megasync_$MEGASYNC_VERSION.tar.gz


# make sure the source tree is in "clean" state
cd ../Source/MEGAShellExtNautilus/
make distclean || true
cd ../../build

export EXT_VERSION=1.0
export EXT_NAME=nautilus-megasync-$EXT_VERSION
rm -rf $EXT_NAME.tar.gz
rm -rf $EXT_NAME
mkdir $EXT_NAME
ln -s ../MEGASync/MEGAShellExtNautilus/nautilus-megasync.spec $EXT_NAME/megasync.spec
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_client.c $EXT_NAME/mega_ext_client.c
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_client.h $EXT_NAME/mega_ext_client.h
ln -s ../../Source/MEGAShellExtNautilus/mega_ext_module.c $EXT_NAME/mega_ext_module.c
ln -s ../../Source/MEGAShellExtNautilus/mega_notify_client.h $EXT_NAME/mega_notify_client.h
ln -s ../../Source/MEGAShellExtNautilus/mega_notify_client.c $EXT_NAME/mega_notify_client.c
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExt.c $EXT_NAME/MEGAShellExt.c
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExt.h $EXT_NAME/MEGAShellExt.h
ln -s ../../Source/MEGAShellExtNautilus/MEGAShellExtNautilus.pro $EXT_NAME/MEGAShellExtNautilus.pro
ln -s ../../Source/MEGAShellExtNautilus/data $EXT_NAME/data
tar cvzfh $EXT_NAME.tar.gz $EXT_NAME
rm -rf $EXT_NAME

# transform arch name, to satisfy Debian requirements
mv $EXT_NAME.tar.gz MEGASync/MEGAShellExtNautilus/nautilus-megasync_$EXT_VERSION.tar.gz
