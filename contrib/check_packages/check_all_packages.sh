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

while getopts ":ikcp:" opt; do
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
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL CENTOS_7;$BASEURLRPM/CentOS_7" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_19;$BASEURLRPM/Fedora_19" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_20;$BASEURLRPM/Fedora_20" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_21;$BASEURLRPM/Fedora_21" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_22;$BASEURLRPM/Fedora_22" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL FEDORA_23;$BASEURLRPM/Fedora_23" #(creo) #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_12.2;$BASEURLRPM/openSUSE_12.2" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_12.3;$BASEURLRPM/openSUSE_12.3" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL OPENSUSE_13.2;$BASEURLRPM/openSUSE_13.2" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_12.04;$BASEURLDEB/xUbuntu_12.04"	 #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_13.10;$BASEURLDEB/xUbuntu_13.10" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_14.04;$BASEURLDEB/xUbuntu_14.04" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_15.04;$BASEURLDEB/xUbuntu_15.04" # OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL UBUNTU_15.10;$BASEURLDEB/xUbuntu_15.10" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL LINUXMINT_17.3;$BASEURLDEB/xUbuntu_14.04" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_7.8.0;$BASEURLDEB/Debian_7.0" #OK
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_8;$BASEURLDEB/Debian_8.0" # OK 
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_9_CLEAN_testing;$URLDEB9" #OK #NOTICE: using other repo
PAIRSVMNAMEREPOURL="$PAIRSVMNAMEREPOURL DEBIAN_9_i386_T;$URLDEB9" #OK #NOTICE: using other repo

##Opensuse 13.1 -> FALLIDO. no hay interfaz de red!!
##Ubuntu 12.10 -> FALLIDO. NO CONSIGO METER curl, no logro meter paquetes (soporte finalizado!)
##Debian6 -> FALLIDO. NO CONSIGO METER curl, soporte finalizo en feb 2016!!!

#~ openSUSE_12.3/ FAIL
#~ openSUSE_Leap_42.1/ #NO
#~ openSUSE_Tumbleweed/ #NO
#~ RHEL_7/ #NO
#~ xUbuntu_12.10/ #FAIL
#~ xUbuntu_13.04/ #NO
#~ xUbuntu_14.10/ #NO
#~ Windows7 !!






for i in $PAIRSVMNAMEREPOURL; do

	VMNAME=`echo $i | cut -d";" -f1`;
	REPO=`echo $i | cut -d";" -f2`;
	
	rm ${VMNAME}_{OK,FAIL} 2> /dev/null
	echo /mnt/DATA/datos/assets/check_packages/check_package.sh $arg_passwd $flag_require_change $flag_remove_megasync $flag_quit_machine $VMNAME $REPO 
	( /mnt/DATA/datos/assets/check_packages/check_package.sh $arg_passwd $flag_require_change $flag_remove_megasync $flag_quit_machine $VMNAME $REPO 2>&1 ) > output_check_package_${VMNAME}.log &

done

#~ #rpm repos
#~ for i in CentOS_7 Fedora_20 Fedora_21 openSUSE_12.2; do

	#~ VMNAME=`echo $i | tr [a-z] [A-Z]`;

	#~ rm ${VMNAME}_{OK,FAIL}
	#~ ( /mnt/DATA/datos/assets/check_packages/check_package.sh -i $VMNAME http://192.168.122.1:8000/RPM/$i 2>&1 ) > salida_check_package_$i.log &

#~ done



#~ #deb repos
#~ for i in Debian_7.0; do

	#~ VMNAME=`echo $i | tr [a-z] [A-Z]`;
	#~ if [ $i == "Debian_7.0" ]; then
		#~ VMNAME="DEBIAN_7.8.0"
		#~ echo " VMNAME corresponding to $i is $VMNAME" 
	#~ fi	

	#~ rm ${VMNAME}_{OK,FAIL}
	#~ echo /mnt/DATA/datos/assets/check_packages/check_package.sh -i $VMNAME http://192.168.122.1:8000/DEB/$i > salida_check_package_$i.log 
	#~ ( /mnt/DATA/datos/assets/check_packages/check_package.sh -ik $VMNAME http://192.168.122.1:8000/DEB/$i 2>&1 ) > salida_check_package_$i.log &

#~ done
