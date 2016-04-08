#!/bin/bash

##
 # @file contrib/checkrepo.sh
 # @brief Checks signature of rpms, and release files in a repository and prints hash of the entire contents
 #           Receives as parameter the path of the repository
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

# Check signatures of RPM repos
find $1 -type f -iname "repomd.xml.asc" -exec sh -c 'if ! gpg --verify $0 2> /dev/null; then echo "Error checking signature of $0"; fi' {} \; 

# Check signatures of DEB repos
find $1 -type f -iname "Release" -exec sh -c 'if ! gpg --verify $0.gpg $0 2> /dev/null; then echo "Error checking signature of $0"; fi' {} \;

# Check signatures of all RPM packages
find $1 -iname "*.rpm" -exec sh -c 'if ! rpm -K $0 > /dev/null; then echo "Error checking signature of $0"; fi' {} \;

#Generate a hash of the whole repo
echo "Repository hash: "
find $1 -type f -print0 | sort -z | xargs -0 shasum | cut -d' ' -f1 | shasum

