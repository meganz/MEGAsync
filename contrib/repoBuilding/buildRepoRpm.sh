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

# cleanup existing repository files
rm -f repodata

#sign packages
for i in `find $DIR -name "*rpm"`; do sudo sign -r -P /srv/obs/projects/RPM.pkg/_signkey $i; done


createrepo -q -c $DIR/repocache --checksum=sha256 --unique-md-filenames --no-database --repo 'Megasync for Fedora 24' --changelog-limit 20 $DIR

#publish pub key
cp  /srv/obs/projects/RPM.pkg/_pubkey ./repodata/repomd.xml.key


#sign repo xml
sudo sign -P /srv/obs/projects/RPM.pkg/_signkey -d ./repodata/repomd.xml
sudo chown pol:users ./repodata/repomd.xml.asc
