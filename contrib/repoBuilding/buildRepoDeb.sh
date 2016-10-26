#!/bin/bash

##
 # @file contrib/repoBuilding/buildRepoDeb.sh
 # @brief
 #
 # (c) 2013-2016 by Mega Limited, Auckland, New Zealand
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


DIR=${1}
DIST=${2}
ARCHS=`find * -maxdepth 0 -type d | paste -s -d" "`
FECHA=`date -Ru`

# cleanup existing repository files
rm -f Packages Packages.gz Release
#rm -fr dists

# create Packages and Sources files
dpkg-scanpackages -m . > Packages
gzip -c9 < Packages  > Packages.gz
dpkg-scansources . > Sources
gzip -c9 Sources > Sources.gz

# create Release file
cat > Release <<-EOF
Archive: $DIST
Codename: $DIST
Origin: manual_build
Label: DEB
Architectures: $ARCHS
Date: $FECHA
Description: Debian repository created by buildRepoDeb
Components: main
EOF

echo "MD5sum:" >> Release
for file in Packages* ; do
	SUM=( $(md5sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done
for file in Sources* ; do
	SUM=( $(md5sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done

echo "SHA1:" >> Release
for file in Packages* ; do
	SUM=( $(sha1sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done
for file in Sources* ; do
	SUM=( $(sha1sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done

echo "SHA256:" >> Release
for file in Packages* ; do
	SUM=( $(sha256sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done
for file in  Sources* ; do
	SUM=( $(sha256sum ${file}) )
	SIZE=$(stat -c '%s' ${file})
	echo " ${SUM} ${SIZE} ${file}" >> Release
done

cp  /srv/obs/projects/DEB.pkg/_pubkey ./Release.key

sudo sign -P /srv/obs/projects/DEB.pkg/_signkey -d Release
mv Release.asc Release.gpg
sudo chown pol:users Release.gpg
