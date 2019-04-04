#!/bin/bash

##
 # @file contrib/getlatestversionAndTriggerBuildAllRepos.sh
 # @brief Gets the latest version of the git project and creates tarball, then
 #     triggers OBS compilation for configured repositories          
 #     It stores the project files at                                
 #         /datos/building/local_project/desktop_$THEDATE   
 #     and the stuff for repos building at                           
 #         /datos/building/osc_projects/$THEDATE    
 #
 # (c) 2013-2017 by Mega Limited, Auckland, New Zealand
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


function printusage {
	echo "$0 [--home] [package=PACKAGE] [user@remoteobsserver] [-t branch] [PROJECTPATH [OSCFOLDER]]"
	echo "This scripts gets latest version of the git project and creates tarball, then triggers OBS compilation for configured repositories"
	echo " An alternative branch/commit can be specified with -t BRANCH "
	echo " Use --home to only update home:Admin project. Otherwise DEB and RPM projects will be updated" 
	echo " Use package=PACKAGE to build only a particular package. Otherwise all the packages will be added" 
}

if [[ $1 == "--help" ]]; then
	printusage
	exit 1
fi


if [[ $1 == "--home" ]]; then
	onlyhomeproject=$1;
	shift
fi

if [[ "$1" == "package="* ]]; then
	packages="$1"
	shift
else
	packages=""
fi

sshpasscommand=""
if [[ $1 == *@* ]]; then
remote=$1
shift
echo -n $remote Password: 
read -s password
sshpasscommand="sshpass -p $password ssh $remote"
$sshpasscommand hostname > /dev/null
if [[ $? != 0 ]]; then
echo
echo unable to connect to $remote, ensure password is ok
exit 1
fi
echo
fi

while getopts ":t:" opt; do
  case $opt in
    t)
		tagtodl="--branch $OPTARG"
      ;;
    #~ \?)
		#~ echo "Invalid option: -$OPTARG" >&2
		#~ display_help $0
		#~ exit
	#~ ;;
	#~ *)
		#~ display_help $0
		#~ exit
	#~ ;;
  esac
done

shift $(($OPTIND-1))

THEDATE=`date +%Y%m%d%H%M%S`
PROJECT_PATH=$1
NEWOSCFOLDER_PATH=$2
if [ -z "$PROJECT_PATH" ]; then
	PROJECT_PATH=/datos/building/local_project/desktop_$THEDATE
	echo "using default PROJECT_PATH: $PROJECT_PATH"
fi

function copy
{
	if [ "$sshpasscommand" ]; then
		sshpass -p $password scp $1 $remote:$2
	else #linking will be enough
		ln -sf $1 $2
	fi
}

#checkout master project and submodules
echo git clone $tagtodl --recursive --depth 1 https://github.com/meganz/desktop $PROJECT_PATH
if ! git clone $tagtodl --recursive --depth 1 https://github.com/meganz/desktop $PROJECT_PATH; then exit 1;fi
pushd $PROJECT_PATH
pushd build

./create_tarball.sh
ver=`cat version`
copy ./MEGAsync/MEGAShellExtDolphin/debian.changelog /assets/changelogs/dolphin-megasync/dolphin-megasync_${ver}.changelog
copy ./MEGAsync/MEGAShellExtNautilus/debian.changelog /assets/changelogs/nautilus-megasync/nautilus-megasync_${ver}.changelog
copy ./MEGAsync/MEGAShellExtNemo/debian.changelog /assets/changelogs/nemo-megasync/nemo-megasync_${ver}.changelog
copy ./MEGAsync/MEGAShellExtThunar/debian.changelog /assets/changelogs/thunar-megasync/thunar-megasync_${ver}.changelog
copy ./MEGAsync/MEGAsync/debian.changelog /assets/changelogs/megasync/megasync_${ver}.changelog

popd
popd

# trigger build commiting new changes into OBS projects
if [ -z "$NEWOSCFOLDER_PATH" ]; then
	NEWOSCFOLDER_PATH=/datos/building/osc_projects/$THEDATE
	echo "using default NEWOSCFOLDER_PATH: $NEWOSCFOLDER_PATH"
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REMOTEP=$password $DIR/triggerBuild.sh $onlyhomeproject $packages $remote $PROJECT_PATH $NEWOSCFOLDER_PATH

