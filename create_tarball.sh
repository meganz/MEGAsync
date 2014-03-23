#!/bin/sh

export MEGASYNC_VERSION=1.0
rm -rf megasync-$MEGASYNC_VERSION.tar.gz
rm -rf megasync-$MEGASYNC_VERSION
mkdir megasync-$MEGASYNC_VERSION
ln -s ../megasync.spec megasync-$MEGASYNC_VERSION/megasync.spec
ln -s ../Source/configure megasync-$MEGASYNC_VERSION/configure
ln -s ../Source/MEGA.pro megasync-$MEGASYNC_VERSION/MEGA.pro
ln -s ../Source/MEGASync megasync-$MEGASYNC_VERSION/MEGASync
ln -s ../Source/MEGAShellExtNautilus megasync-$MEGASYNC_VERSION/MEGAShellExtNautilus
tar cvzfh megasync-$MEGASYNC_VERSION.tar.gz -X Source/.gitignore -X Source/MEGASync/.gitignore -X Source/MEGASync/sdk/.gitignore --exclude megasync-$MEGASYNC_VERSION/MEGASync/sdk/sqlite3.c megasync-$MEGASYNC_VERSION
rm -rf megasync-$MEGASYNC_VERSION


