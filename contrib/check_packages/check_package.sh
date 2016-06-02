#!/bin/bash

##
 # @file contrib/check_packages/check_package.sh
 # @brief checks the correctness of a package using a virtual machine
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

display_help() {
    local app=$(basename "$0")
    echo ""
    echo "Usage:"
    echo " $app [-c] [-i] [-k] [-p pass] [-x pathXMLdir] VMNAME URL_REPO"
    echo ""
    echo "This script will check the correctness of a package using a virtual machine."
    echo " It sill receive the machine name and the repository that will be used to download" 
    echo " megasync packages."
    echo ""
    echo "If the repo has a newly generated package, we can use -c to only validate a VM"
    echo " in case the version of megasync package has changed. This will fail if the VM already"
    echo " contained the latests megasync package" 
    echo " In case we dont know the state of the VM, we can safely run the test using -c and -i"
    echo ""
    echo "This script generates 2 files: "
    echo " - Either ${VMNAME}_OK (in case of success) or ${VMNAME}_FAIL (in case of failure)"
    echo " - result_${VMNAME}.log: this file sumarizes the problems encountered while testing."
    echo ""
    echo "Options:"
    echo " -c : check megasync package has changed (updated or newly installed)"
    echo " -i : install anew (removes previous megasync package)"
    echo " -k : keep VM running after completion"
    echo " -p pass : password for VM (both user mega & root)"
    echo " -x pathXMLdir : path for the xml files describing the VMs"
    echo ""
}

remove_megasync=0
quit_machine=1
require_change=0
pathXMLdir=/mnt/DATA/datos/assets/check_packages

while getopts ":ikcp:x:" opt; do
  case $opt in
    i)
		remove_megasync=1
      ;;
	c)
		require_change=1
	;;
	p)
		arg_passwd="-p $OPTARG"
		sshpasscommand="sshpass $arg_passwd"
	;;	
    k)
		quit_machine=0
      ;;
    x)
		pathXMLdir="$OPTARG"
      ;;      
    \?)
		echo "Invalid option: -$OPTARG" >&2
		display_help $0
		exit
	;;
	*)
		display_help $0
		exit
	;;
  esac
done

shift $(($OPTIND-1))

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    display_help $0
    exit
fi

VMNAME=$1
REPO=$2


echo "" > result_$VMNAME.log

logLastComandResult() {
	local RESULT=$?
	logOperationResult "$1" $RESULT
}

logOperationResult(){
	local RESULT=$2
	if [ $RESULT -eq 0 ]; then
		RESULT="OK"
	else
		RESULT="FAIL($RESULT)"
	fi
	
	printf "%-50s %s\n"  "$1" "$RESULT" >> result_$VMNAME.log
}

logSth(){
	printf "%-50s %s\n"  "$1" "$2" >> result_$VMNAME.log
}



echo " running machine $VMNAME ..."
sudo virsh create $pathXMLdir/$VMNAME.xml 

#sudo virsh domiflist $VMNAME 
IP_GUEST=$( sudo arp -n | grep `sudo virsh domiflist $VMNAME  | grep vnet | awk '{print $NF}'` | awk '{print $1}' )
while [ -z $IP_GUEST ]; do
echo " could not get guest IP. retrying in 2 sec ..."
sleep 2
IP_GUEST=$( sudo arp -n | grep `sudo virsh domiflist $VMNAME  | grep vnet | awk '{print $NF}'` | awk '{print $1}' )
done

echo " obtained IP guest = $IP_GUEST"

echo " sshing to save host into the known hosts ...."
#sshpass would do that automatically neither ask for it, we need to force it the first time
while ! $sshpasscommand ssh  -oStrictHostKeyChecking=no root@$IP_GUEST hostname ; do 
echo " could ssh into GUEST. retrying in 2 sec ..."
sleep 2
done
 

echo " deleting testing file ..."
$sshpasscommand ssh root@$IP_GUEST rm /home/mega/testFile.txt #Notice: filename comes from the shared file
echo " checking existing megasync running ..."
$sshpasscommand ssh root@$IP_GUEST ps aux | grep megasync
echo " killing megasync ..."
$sshpasscommand ssh root@$IP_GUEST killall megasync


#DEPENDENT ON SYSTEM
if [ $VMNAME == "OPENSUSE_12.3" ]; then
	#this particular VM had a problem with missing DNS servers.
	$sshpasscommand ssh root@$IP_GUEST netconfig update -f #TODO: take this out and configure the machine accordingly
fi

if [[ $VMNAME == *"OPENSUSE"* ]]; then
	#stop packagekit service to prevent zypper from freezing
	$sshpasscommand ssh root@$IP_GUEST service packagekit stop
	sleep 1

	$sshpasscommand ssh root@$IP_GUEST rpm --rebuilddb #TODO: test without this in opensuses > 12.2

	if [ $remove_megasync -eq 1 ]; then
		echo " removing megasync ..."
		$sshpasscommand ssh root@$IP_GUEST zypper --non-interactive remove megasync
		logLastComandResult "removing megasync ..."
	fi
	
	$sshpasscommand ssh root@$IP_GUEST "cat > /etc/zypp/repos.d/megasync.repo" <<-EOF
	[MEGAsync]
	name=MEGAsync
	type=rpm-md
	baseurl=$REPO/
	gpgkey=$REPO/repodata/repomd.xml.key
	gpgcheck=1
	autorefresh=1
	enabled=1
	EOF
	$sshpasscommand ssh root@$IP_GUEST zypper --non-interactive refresh 2> tmp$VMNAME
	resultMODREPO=$?
	#notice: zypper will report 0 as status even though it "failed", we do stderr checking
	if cat tmp$VMNAME | grep $REPO; then
	 resultMODREPO=$((1000 + 0$resultMODREPO)); cat tmp$VMNAME; 
	else
	 resultMODREPO=0 #we discard any other failure
	fi; rm tmp$VMNAME;	
	#~ if [ -s tmp$VMNAME ]; then resultMODREPO=$((1000 + $resultMODREPO)); cat tmp$VMNAME; fi; rm tmp$VMNAME;	
	logOperationResult "modifying repos ..." $resultMODREPO
	cat /etc/zypp/repos.d/megasync.repo
	
	echo " reinstalling/updating megasync ..."
	BEFOREINSTALL=`$sshpasscommand ssh root@$IP_GUEST rpm -q megasync`
	attempts=3
	$sshpasscommand ssh root@$IP_GUEST zypper --non-interactive install -f megasync 
	resultINSTALL=$?
	while [ $attempts -ge 0 ] || $resultINSTALL ; do
		$sshpasscommand ssh root@$IP_GUEST zypper --non-interactive install -f megasync 
		resultINSTALL=$?
		$attempts=$(($attempts - 1))
	done
	#TODO: zypper might fail and still say "IT IS OK!"
	#Doing stderr checking will give false FAILS, since zypper outputs non failure stuff in stderr
	#if [ -s tmp$VMNAME ]; then resultINSTALL=$((1000 + $resultINSTALL)); cat tmp$VMNAME; fi; rm tmp$VMNAME;	
	AFTERINSTALL=`$sshpasscommand ssh root@$IP_GUEST rpm -q megasync`
	resultINSTALL=$(($? + 0$resultINSTALL))
	echo "BEFOREINSTALL = $BEFOREINSTALL"
	echo "AFTERINSTALL = $AFTERINSTALL"
	if [ $require_change -eq 1 ]; then
		if [ "$BEFOREINSTALL" == "$AFTERINSTALL" ]; then resultINSTALL=$((1000 + 0$resultINSTALL)); fi
	fi
	logOperationResult "reinstalling/updating megasync ..." $resultINSTALL
	VERSIONINSTALLEDAFTER=`echo $AFTERINSTALL	| grep megasync | awk '{for(i=1;i<=NF;i++){ if(match($i,/[0-9].[0-9].[0-9]/)){print $i} } }'`
	logSth "installed megasync ..." "$VERSIONINSTALLEDAFTER"
		
elif [[ $1 == *"DEBIAN"* ]] || [[ $1 == *"UBUNTU"* ]] || [[ $1 == *"LINUXMINT"* ]]; then

	if [ $remove_megasync -eq 1 ]; then
		echo " removing megasync ..."
		$sshpasscommand ssh root@$IP_GUEST DEBIAN_FRONTEND=noninteractive apt-get -y remove megasync
		logLastComandResult "removing megasync ..."
	fi
	sleep 1

	echo " modifying repos ..."
	$sshpasscommand ssh root@$IP_GUEST "cat > /etc/apt/sources.list.d/megasync.list" <<-EOF
	deb $REPO/ ./
	EOF
	$sshpasscommand ssh root@$IP_GUEST DEBIAN_FRONTEND=noninteractive apt-get -y update 2> tmp$VMNAME
	resultMODREPO=$?
	#notice: zypper will report 0 as status even though it "failed", we do stderr checking
	if cat tmp$VMNAME | grep $REPO; then
	 resultMODREPO=$((1000 + 0$resultMODREPO)); cat tmp$VMNAME; 
	#~ else
	 #~ resultMODREPO=0 #we discard any other failure
	fi; rm tmp$VMNAME;	
	logOperationResult "modifying repos ..." $resultMODREPO
	$sshpasscommand ssh root@$IP_GUEST cat /etc/apt/sources.list.d/megasync.list

	
	
	echo " reinstalling/updating megasync ..."
	BEFOREINSTALL=`$sshpasscommand ssh root@$IP_GUEST dpkg -l megasync`
	$sshpasscommand ssh root@$IP_GUEST DEBIAN_FRONTEND=noninteractive apt-get -y install megasync
	resultINSTALL=$?
	AFTERINSTALL=`$sshpasscommand ssh root@$IP_GUEST dpkg -l megasync`
	resultINSTALL=$(($? + 0$resultINSTALL))
	echo "BEFOREINSTALL = $BEFOREINSTALL"
	echo "AFTERINSTALL = $AFTERINSTALL"
	if [ $require_change -eq 1 ]; then
		if [ "$BEFOREINSTALL" == "$AFTERINSTALL" ]; then resultINSTALL=$((1000 + 0$resultINSTALL)); fi
	fi
	logOperationResult "reinstalling/updating megasync ..." $resultINSTALL	
	VERSIONINSTALLEDAFTER=`echo $AFTERINSTALL	| grep megasync | awk '{for(i=1;i<=NF;i++){ if(match($i,/[0-9].[0-9].[0-9]/)){print $i} } }'`
	logSth "installed megasync ..." "$VERSIONINSTALLEDAFTER"

elif [[ $1 == *"ARCHLINUX"* ]]; then

	if [ $remove_megasync -eq 1 ]; then
		echo " removing megasync ..."
		$sshpasscommand ssh root@$IP_GUEST pacman -Rc --noconfirm megasync
		logLastComandResult "removing megasync ..."
	fi
	sleep 1

	echo " modifying repos ..."
	
	#remove repo if existing
	$sshpasscommand ssh root@$IP_GUEST sed -n "'1h;1!H;\${g;s/###REPO for MEGA###\n.*###END REPO for MEGA###//;p;}'" -i /etc/pacman.conf 

	#include repo
	$sshpasscommand ssh root@$IP_GUEST "cat >> /etc/pacman.conf" <<-EOF
###REPO for MEGA###
[DEB_Arch_Extra]
SigLevel = Optional TrustAll
Server = $REPO/\$arch
###END REPO for MEGA###
EOF
	
	$sshpasscommand ssh root@$IP_GUEST pacman -Sy --noconfirm 2> tmp$VMNAME
	resultMODREPO=$?
	#notice: in case pacman reports 0 as status even though it "failed", we do stderr checking
	if cat tmp$VMNAME | grep $REPO; then
	 resultMODREPO=$((1000 + 0$resultMODREPO)); cat tmp$VMNAME; 
	#~ else
	 #~ resultMODREPO=0 #we discard any other failure
	fi; rm tmp$VMNAME;	
	logOperationResult "modifying repos ..." $resultMODREPO
	$sshpasscommand ssh root@$IP_GUEST "cat /etc/pacman.conf | grep 'REPO for MEGA' -A 5"
	
	
	echo " reinstalling/updating megasync ..."
	BEFOREINSTALL=`$sshpasscommand ssh root@$IP_GUEST pacman -Q megasync`
	$sshpasscommand ssh root@$IP_GUEST pacman -S --noconfirm megasync
	resultINSTALL=$?
	AFTERINSTALL=`$sshpasscommand ssh root@$IP_GUEST pacman -Q megasync`
	resultINSTALL=$(($? + 0$resultINSTALL))
	echo "BEFOREINSTALL = $BEFOREINSTALL"
	echo "AFTERINSTALL = $AFTERINSTALL"
	if [ $require_change -eq 1 ]; then
		if [ "$BEFOREINSTALL" == "$AFTERINSTALL" ]; then resultINSTALL=$((1000 + 0$resultINSTALL)); fi
	fi
	logOperationResult "reinstalling/updating megasync ..." $resultINSTALL	
	VERSIONINSTALLEDAFTER=`echo $AFTERINSTALL	| grep megasync | awk '{for(i=1;i<=NF;i++){ if(match($i,/[0-9].[0-9].[0-9]/)){print $i} } }'`
	logSth "installed megasync ..." "$VERSIONINSTALLEDAFTER"


else
	#stop packagekit service to prevent yum from freezing
		if [[ $VMNAME == *"FEDORA"* ]]; then
		$sshpasscommand ssh root@$IP_GUEST service packagekit stop
	fi
	sleep 1
	
	YUM=yum
	if $sshpasscommand ssh root@$IP_GUEST which dnf; then
		YUM="dnf --best"
	fi

	if [ $remove_megasync -eq 1 ]; then
		echo " removing megasync ..."
		$sshpasscommand ssh root@$IP_GUEST $YUM -y --disableplugin=refresh-packagekit remove megasync
		logLastComandResult "removing megasync ..."
	fi
	sleep 1
	

	echo " modifying repos ..."
	$sshpasscommand ssh root@$IP_GUEST "cat > /etc/yum.repos.d/megasync.repo" <<-EOF
	[MEGAsync]
	name=MEGAsync
	baseurl=$REPO/
	gpgkey=$REPO/repodata/repomd.xml.key
	gpgcheck=1
	enabled=1
	EOF
	$sshpasscommand ssh root@$IP_GUEST $YUM -y --disableplugin=refresh-packagekit clean all 2> tmp$VMNAME
	resultMODREPO=$?
	#fix inconclusive problem in Fedora24:
	cat tmp$VMNAME | grep -v "FutureWarning: split() requires a non-empty pattern match\|return _compile(pattern, flags).split(stri" > tmp$VMNAME
	if [ -s tmp$VMNAME ]; then resultMODREPO=$((1000 + $resultMODREPO)); cat tmp$VMNAME; fi; rm tmp$VMNAME; 
	logOperationResult "modifying repos ..." $resultMODREPO
	
	cat /etc/yum.repos.d/megasync.repo
	sleep 1
	
	
	echo " reinstalling/updating megasync ..."
	BEFOREINSTALL=`$sshpasscommand ssh root@$IP_GUEST rpm -q megasync`
	$sshpasscommand ssh root@$IP_GUEST $YUM -y --disableplugin=refresh-packagekit install megasync  2> tmp$VMNAME
	resultINSTALL=$(($? + 0$resultINSTALL)) #TODO: yum might fail and still say "IT IS OK!"
	#Doing simple stderr checking will give false FAILS, since yum outputs non failure stuff in stderr
	if cat tmp$VMNAME | grep $REPO; then
	 resultINSTALL=$((1000 + 0$resultINSTALL)); cat tmp$VMNAME; 
	fi; rm tmp$VMNAME;	
	
	AFTERINSTALL=`$sshpasscommand ssh root@$IP_GUEST rpm -q megasync`
	resultINSTALL=$(($? + 0$resultINSTALL))
	echo "BEFOREINSTALL = $BEFOREINSTALL"
	echo "AFTERINSTALL = $AFTERINSTALL"
	if [ $require_change -eq 1 ]; then
		if [ "$BEFOREINSTALL" == "$AFTERINSTALL" ]; then resultINSTALL=$((1000 + 0$resultINSTALL)); fi
	fi
	logOperationResult "reinstalling/updating megasync ..." $resultINSTALL
	VERSIONINSTALLEDAFTER=`echo $AFTERINSTALL	| grep megasync | awk '{for(i=1;i<=NF;i++){ if(match($i,/[0-9].[0-9].[0-9]/)){print $i} } }'`
	logSth "installed megasync ..." "$VERSIONINSTALLEDAFTER"
fi
#END OF DEPENDENT OF SYSTEM

theDisplay="DISPLAY=:0.0"

echo " relaunching megasync as user ..."
VERSIONMEGASYNCREMOTERUNNING=`$sshpasscommand ssh -oStrictHostKeyChecking=no  mega@$IP_GUEST $theDisplay megasync --version | grep -i megasync | awk '{for(i=1;i<=NF;i++){ if(match($i,/[0-9].[0-9].[0-9]/)){print $i} } }'`
logSth "running megasync ..." "$VERSIONMEGASYNCREMOTERUNNING"

$sshpasscommand ssh -oStrictHostKeyChecking=no  mega@$IP_GUEST $theDisplay megasync &
sleep 5 #TODO: sleep longer?

echo " checking new megasync running ..."
$sshpasscommand ssh root@$IP_GUEST ps aux | grep megasync
resultRunning=$?
logOperationResult "checking new megasync running ..." $resultRunning

echo " forcing POST to dl test file ..."
$sshpasscommand ssh root@$IP_GUEST "curl 'https://127.0.0.1:6342/' -H 'Origin: https://mega.nz' --data-binary '{\"a\":\"l\",\"h\":\"FQ5miCCB\",\"k\":\"WkMOvzgPWhBtvE7tYQQv8urhwuYmuS74C3HnhboDE-I\"}' --compressed --insecure"
sleep 20 #TODO: sleep longer?

echo " check file dl correctly ..."
$sshpasscommand ssh root@$IP_GUEST cat /home/mega/testFile.txt >/dev/null  #TODO: do hash file comparation
resultDL=$?
logOperationResult "check file dl correctly ..." $resultDL

if [ $resultDL -eq 0 ] && [ $resultRunning -eq 0 ] \
&& [ $resultMODREPO -eq 0 ] && [ $resultINSTALL -eq 0 ]; then
	echo " megasync working smoothly" 
	touch ${VMNAME}_OK
else
	echo "MEGASYNC FAILED: $resultDL $resultRunning $resultMODREPO $resultINSTALL"
	#cat result_$VMNAME.log
	touch ${VMNAME}_FAIL
fi

echo " check repo set ok ..."
$sshpasscommand ssh root@$IP_GUEST  "cat /usr/share/doc/megasync/{distro,version}" 
$sshpasscommand ssh root@$IP_GUEST  "cat /etc/apt/sources.list.d/megasync.list" 

	
if [ $quit_machine -eq 1 ]; then
	$sshpasscommand ssh root@$IP_GUEST shutdown -h now & #unfurtonately this might (though rarely) hang if vm destroyed
	#$sshpasscommand ssh root@$IP_GUEST sleep 20 #unfurtonately this might hang if vm destroyed
	sleep 8
	sudo virsh destroy $VMNAME
fi
