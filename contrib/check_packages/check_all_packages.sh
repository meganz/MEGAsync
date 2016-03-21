#!/bin/bash

##
 # @file contrib/check_packages/check_all_packages.sh
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

remove_megasync=0
quit_machine=1


while getopts ":ikcp:x:" opt; do
  case $opt in
    i)
		remove_megasync=1
		flag_remove_megasync="-$opt"
      ;;
	c)
		flag_require_change="-$opt"
	;;
	p)
		arg_passwd="-$opt $OPTARG"
	;;
	x)
		flagXMLdir="-$opt $OPTARG"
      ;;     		
    k)
		quit_machine=0
		flag_quit_machine="-$opt"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

shift $(($OPTIND-1))


echo -n " Repository packages checking beginning at "
sudo date

# notice: use http://linux.deve.... instead of https://linu...
BASEURL=$1
BASEURLDEB9=$1
BASEURLDEB=$BASEURL
BASEURLRPM=$BASEURL
URLDEB9=$BASEURL/Debian_9.0
if [ -z $BASEURL ]; then
 BASEURL=http://192.168.122.1:8000
 BASEURLDEB=$BASEURL/DEB
 BASEURLRPM=$BASEURL/RPM
 URLDEB9=http://192.168.122.1:8001
fi

PAIRSVMNAMEREPOURL=""
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL CENTOS_7;$BASEURLRPM/CentOS_7"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_19;$BASEURLRPM/Fedora_19"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_20;$BASEURLRPM/Fedora_20"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_21;$BASEURLRPM/Fedora_21"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_22;$BASEURLRPM/Fedora_22"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_23;$BASEURLRPM/Fedora_23"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_12.2;$BASEURLRPM/openSUSE_12.2"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_12.3;$BASEURLRPM/openSUSE_12.3"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_13.2;$BASEURLRPM/openSUSE_13.2"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_12.04;$BASEURLDEB/xUbuntu_12.04"	
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_13.10;$BASEURLDEB/xUbuntu_13.10"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_14.04;$BASEURLDEB/xUbuntu_14.04"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_15.04;$BASEURLDEB/xUbuntu_15.04"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_15.10;$BASEURLDEB/xUbuntu_15.10"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL LINUXMINT_17.3;$BASEURLDEB/xUbuntu_14.04"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_7.8.0;$BASEURLDEB/Debian_7.0"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_8;$BASEURLDEB/Debian_8.0"
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_9_CLEAN_testing;$URLDEB9" #NOTICE: using other repo
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_9_i386_T;$URLDEB9" #NOTICE: using other repo

#existing,but failing VMs
##Opensuse 13.1 -> no network interface!!
##Ubuntu 12.10 -> cannot install curl (support ended, official repos fail)
##Debian6 -> cannot install curl (support ended feb 2016, official repos fail)

#Untested but offered repos 
#~ openSUSE_Leap_42.1/ #NO VM
#~ openSUSE_Tumbleweed/ #NO VM
#~ RHEL_7/ #NO VM
#~ xUbuntu_12.10/ #VM FAILS
#~ xUbuntu_13.04/ #NO VM
#~ xUbuntu_14.10/ #NO VM
# ALL i386 (but Debian9)





for i in $PAIRSVMNAMEREPOURL; do

	VMNAME=`echo $i | cut -d";" -f1`;
	REPO=`echo $i | cut -d";" -f2`;
	
	rm ${VMNAME}_{OK,FAIL} 2> /dev/null
	echo /mnt/DATA/datos/assets/check_packages/check_package.sh $arg_passwd $flagXMLdir $flag_require_change $flag_remove_megasync $flag_quit_machine $VMNAME $REPO 
	( /mnt/DATA/datos/assets/check_packages/check_package.sh $arg_passwd $flagXMLdir $flag_require_change $flag_remove_megasync $flag_quit_machine $VMNAME $REPO 2>&1 ) > output_check_package_${VMNAME}.log &

done
