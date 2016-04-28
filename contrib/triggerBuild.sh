#!/bin/bash

##
 # @file contrib/triggerBuild.sh
 # @brief Triggers OBS compilation for configured repositories 
 #     suppossing project with tarball built at $PROJECT_PATH/    
 #     suposing oscrc configured with apiurl correctly:           
 #      nano ~/.oscrc                                             
 #	     apiurl=https://linux
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

PROJECT_PATH=$1
NEWOSCFOLDER_PATH=$2
if [ -z "$PROJECT_PATH" ]; then
	PROJECT_PATH=/mnt/DATA/datos/assets/local_project/desktop
	echo "using default PROJECT_PATH: $PROJECT_PATH"
fi
if [ -z "$NEWOSCFOLDER_PATH" ]; then
	NEWOSCFOLDER_PATH=/mnt/DATA/datos/building/osc_projects/`date +%Y%m%d%H%M%S`
	echo "using default NEWOSCFOLDER_PATH: $NEWOSCFOLDER_PATH"
fi

export EDITOR=nano

echo "creating folder with OBS projects..."
mkdir $NEWOSCFOLDER_PATH
cd $NEWOSCFOLDER_PATH

echo "checking out existing OBS projects..."
osc co RPM
osc co DEB

for package in MEGAShellExtNautilus MEGAsync; do
	
	echo "deleting old files for package $package ... : "$NEWOSCFOLDER_PATH/{DEB,RPM}/$package/*
	rm $NEWOSCFOLDER_PATH/{DEB,RPM}/$package/*

	echo "replacing files with newly generated  (traball, specs, dsc and so for) for package $package ..."
	ln -sf $PROJECT_PATH/build/MEGAsync/$package/*.spec $NEWOSCFOLDER_PATH/RPM/$package/
	ln -sf $PROJECT_PATH/build/MEGAsync/$package/*tar.gz $NEWOSCFOLDER_PATH/RPM/$package/
	for i in $PROJECT_PATH/build/MEGAsync/$package/{PKGBUILD,*.dsc,*.tar.gz,debian.changelog,debian.control,debian.postinst,debian.postrm,debian.rules} ; do ln -sf $i $NEWOSCFOLDER_PATH/DEB/$package/; done
done

echo "modifying files included/excluded in projects (to respond to e.g. tar.gz version changes)"
osc addremove $NEWOSCFOLDER_PATH/DEB/*
osc addremove $NEWOSCFOLDER_PATH/RPM/* 

echo "updating changed files and hence triggering rebuild in the OBS platform ...."
osc ci $NEWOSCFOLDER_PATH/DEB
osc ci $NEWOSCFOLDER_PATH/RPM


