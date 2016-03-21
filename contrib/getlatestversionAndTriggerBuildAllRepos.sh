#!/bin/bash

##
 # @file contrib/getlatestversionAndTriggerBuildAllRepos.sh
 # @brief Gets the project cloning git project and creates tarball, then
 #     Triggers OSB compilation for configured repositories          
 #     It stores the project files at                                
 #         /mnt/DATA/datos/building/local_project/desktop_$THEDATE   
 #     and the stuff for repos building at                           
 #         /mnt/DATA/datos/building/osc_projects/$THEDATE    
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


THEDATE=`date +%Y%m%d%H%M%S`
PROJECT_PATH=$1
NEWOSCFOLDER_PATH=$2
if [ -z "$PROJECT_PATH" ]; then
	PROJECT_PATH=/mnt/DATA/datos/building/local_project/desktop_$THEDATE
	echo "using default PROJECT_PATH: $PROJECT_PATH"
fi

#checkout master project and submodules
if ! git clone https://github.com/meganz/desktop $PROJECT_PATH; then exit 1;fi
pushd $PROJECT_PATH
git submodule init
git submodule update
pushd build
./create_tarball.sh
popd
popd

# trigger build commiting new changes into OSB projects
if [ -z "$NEWOSCFOLDER_PATH" ]; then
#	NEWOSCFOLDER_PATH=/mnt/DATA/datos/assets/osc
	NEWOSCFOLDER_PATH=/mnt/DATA/datos/building/osc_projects/$THEDATE
	echo "using default NEWOSCFOLDER_PATH: $NEWOSCFOLDER_PATH"
fi

/mnt/DATA/datos/assets/triggerBuild.sh $PROJECT_PATH $NEWOSCFOLDER_PATH
