#!/bin/bash

##
 # @file build/create_tarball.sh
 # @brief Generates MEGAsync and extensions tarballs
 #
 # (c) 2013-2014 by Mega Limited, Auckland, New Zealand
 #
 # This file is part of the MEGA SDK - Client Access Engine.
 #
 # Applications using the MEGA API must present a valid application key
 # and comply with the the rules set forth in the Terms of Service.
 #
 # The MEGA SDK is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 #
 # @copyright Simplified (2-clause) BSD License.
 #
 # You should have received a copy of the license along with this
 # program.
##

set -euo pipefail
IFS=$'\n\t'

# make sure the source tree is in "clean" state
cwd=$(pwd)
cd ../Source
make distclean 2> /dev/null || true
cd MEGASync
make distclean 2> /dev/null || true
cd mega
make distclean 2> /dev/null || true
rm -fr bindings/qt/3rdparty || true
./clean.sh || true
cd $cwd

# download software archives
archives=$cwd/archives
rm -fr $archives
mkdir $archives
../Source/MEGASync/mega/contrib/build_sdk.sh -n -f -w -s -o -v $archives

# get current version
MEGASYNC_VERSION=`grep -Po 'const QString Preferences::VERSION_STRING = QString::fromAscii\("\K[^"]*' ../Source/MEGASync/control/Preferences.cpp`
export MEGASYNC_NAME=megasync-$MEGASYNC_VERSION
rm -rf $MEGASYNC_NAME.tar.gz
rm -rf $MEGASYNC_NAME

echo "MEGAsync version: $MEGASYNC_VERSION"

# delete previously generated files
rm -fr MEGAsync/MEGAsync/megasync_*.dsc
rm -fr MEGAsync/MEGAsync.debug/megasync-debug_*.dsc
# fix version number in template files and copy to appropriate directories
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync/megasync.spec > MEGAsync/MEGAsync/megasync.spec
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync/megasync.dsc > MEGAsync/MEGAsync/megasync_$MEGASYNC_VERSION.dsc
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync/PKGBUILD > MEGAsync/MEGAsync/PKGBUILD

sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync.debug/megasync-debug.spec > MEGAsync/MEGAsync.debug/megasync-debug.spec
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync.debug/megasync-debug.dsc > MEGAsync/MEGAsync.debug/megasync-debug_$MEGASYNC_VERSION.dsc
sed -e "s/MEGASYNC_VERSION/$MEGASYNC_VERSION/g" templates/MEGAsync/PKGBUILD > MEGAsync/MEGAsync.debug/PKGBUILD

# read the last generated ChangeLog version
version_file="version"

if [ -s "$version_file" ]; then
    last_version=$(cat "$version_file")
else
    last_version="none"
fi

if [ "$last_version" != "$MEGASYNC_VERSION" ]; then
    # add RPM ChangeLog entry
    changelog="MEGAsync/MEGAsync/megasync.changes"
    changelogold="MEGAsync/MEGAsync/megasync.changes.old"
    if [ -f $changelog ]; then
        mv $changelog $changelogold
    fi
    ./generate_rpm_changelog_entry.sh ../Source/MEGASync/control/Preferences.cpp > $changelog
    if [ -f $changelogold ]; then
        cat $changelogold >> $changelog
        rm $changelogold
    fi

    # add DEB ChangeLog entry
    changelog="MEGAsync/MEGAsync/debian.changelog"
    changelogold="MEGAsync/MEGAsync/debian.changelog.old"
    if [ -f $changelog ]; then
        mv $changelog $changelogold
    fi
    ./generate_deb_changelog_entry.sh $MEGASYNC_VERSION ../Source/MEGASync/control/Preferences.cpp > $changelog
    if [ -f $changelogold ]; then
        cat $changelogold >> $changelog
        rm $changelogold
    fi

    # update version file
    echo $MEGASYNC_VERSION > $version_file
fi

# create archive
mkdir $MEGASYNC_NAME
ln -s ../MEGAsync/MEGAsync/megasync.spec $MEGASYNC_NAME/megasync.spec
ln -s ../MEGAsync/MEGAsync/debian.postinst $MEGASYNC_NAME/debian.postinst
ln -s ../MEGAsync/MEGAsync/debian.postrm $MEGASYNC_NAME/debian.postrm
ln -s ../../Source/configure $MEGASYNC_NAME/configure
ln -s ../../Source/MEGA.pro $MEGASYNC_NAME/MEGA.pro
ln -s ../../Source/MEGASync $MEGASYNC_NAME/MEGASync
ln -s $archives $MEGASYNC_NAME/archives
tar czfh $MEGASYNC_NAME.tar.gz $MEGASYNC_NAME
rm -rf $MEGASYNC_NAME

# delete any previous archive
rm -fr MEGAsync/MEGAsync/megasync_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $MEGASYNC_NAME.tar.gz MEGAsync/MEGAsync/megasync_$MEGASYNC_VERSION.tar.gz


# create archive for debug
MEGASYNC_NAMED=megasync-debug-$MEGASYNC_VERSION
mkdir $MEGASYNC_NAMED
ln -s ../MEGAsync/MEGAsync.debug/megasync-debug.spec $MEGASYNC_NAMED/
ln -s ../MEGAsync/MEGAsync.debug/debian.postinst $MEGASYNC_NAMED/
ln -s ../MEGAsync/MEGAsync.debug/debian.postrm $MEGASYNC_NAMED/
ln -s ../../Source/configure $MEGASYNC_NAMED/configure
ln -s ../../Source/MEGA.pro $MEGASYNC_NAMED/MEGA.pro
ln -s ../../Source/MEGASync $MEGASYNC_NAMED/MEGASync
ln -s $archives $MEGASYNC_NAMED/archives
tar czfh $MEGASYNC_NAMED.tar.gz $MEGASYNC_NAMED
rm -rf $MEGASYNC_NAMED

# delete any previous archive
rm -fr MEGAsync/MEGAsync.debug/megasync-debug_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $MEGASYNC_NAMED.tar.gz MEGAsync/MEGAsync.debug/megasync-debug_$MEGASYNC_VERSION.tar.gz


#
# Nautilus
#

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
rm -fr MEGAsync/MEGAShellExtNautilus/nautilus-megasync_*.dsc

# fix version number in template files and copy to appropriate directories
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/nautilus-megasync.spec > MEGAsync/MEGAShellExtNautilus/nautilus-megasync.spec
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/nautilus-megasync.dsc > MEGAsync/MEGAShellExtNautilus/nautilus-megasync_$EXT_VERSION.dsc
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtNautilus/PKGBUILD > MEGAsync/MEGAShellExtNautilus/PKGBUILD

# create archive
mkdir $EXT_NAME
ln -s ../MEGAsync/MEGAShellExtNautilus/nautilus-megasync.spec $EXT_NAME/nautilus-megasync.spec
ln -s ../MEGAsync/MEGAShellExtNautilus/debian.postinst $EXT_NAME/debian.postinst
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
rm -fr MEGAsync/MEGAShellExtNautilus/nautilus-megasync_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $EXT_NAME.tar.gz MEGAsync/MEGAShellExtNautilus/nautilus-megasync_$EXT_VERSION.tar.gz


#
# Thunar
#

# make sure the source tree is in "clean" state
cd ../Source/MEGAShellExtThunar/
make distclean 2> /dev/null || true
cd ../../build

# extension uses the same version number as MEGASync app
export EXT_VERSION=$MEGASYNC_VERSION
export EXT_NAME=thunar-megasync-$EXT_VERSION
rm -rf $EXT_NAME.tar.gz
rm -rf $EXT_NAME

# delete previously generated files
rm -fr MEGAsync/MEGAShellExtThunar/thunar-megasync_*.dsc

# fix version number in template files and copy to appropriate directories
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtThunar/thunar-megasync.spec > MEGAsync/MEGAShellExtThunar/thunar-megasync.spec
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtThunar/thunar-megasync.dsc > MEGAsync/MEGAShellExtThunar/thunar-megasync_$EXT_VERSION.dsc
sed -e "s/EXT_VERSION/$EXT_VERSION/g" templates/MEGAShellExtThunar/PKGBUILD > MEGAsync/MEGAShellExtThunar/PKGBUILD

# create archive
mkdir $EXT_NAME
ln -s ../MEGAsync/MEGAShellExtThunar/thunar-megasync.spec $EXT_NAME/thunar-megasync.spec
ln -s ../../Source/MEGAShellExtThunar/mega_ext_client.c $EXT_NAME/mega_ext_client.c
ln -s ../../Source/MEGAShellExtThunar/mega_ext_client.h $EXT_NAME/mega_ext_client.h
ln -s ../../Source/MEGAShellExtThunar/MEGAShellExt.c $EXT_NAME/MEGAShellExt.c
ln -s ../../Source/MEGAShellExtThunar/MEGAShellExt.h $EXT_NAME/MEGAShellExt.h
ln -s ../../Source/MEGAShellExtThunar/MEGAShellExtThunar.pro $EXT_NAME/MEGAShellExtThunar.pro
export GZIP=-9
tar czfh $EXT_NAME.tar.gz --exclude Makefile --exclude '*.o' $EXT_NAME
rm -rf $EXT_NAME

# delete any previous archive
rm -fr MEGAsync/MEGAShellExtThunar/thunar-megasync_*.tar.gz
# transform arch name, to satisfy Debian requirements
mv $EXT_NAME.tar.gz MEGAsync/MEGAShellExtThunar/thunar-megasync_$EXT_VERSION.tar.gz


rm -fr $archives
