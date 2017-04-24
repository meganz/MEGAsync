#!/bin/bash

#USAGE: $0 DLfolder packageName repoName
#after build is complete put changelogs in /srv/obs (see the end of this file): 

backupdate=`date "+%Y%m%d"`
obsbuildpath=/srv/obs/build
obsrepospath=/srv/obs/repos

if [ "$1" ]
then
    DLfolder=$1
    shift
else
DLfolder=/mnt/BACKUP/mega.nz_linux/$backupdate
fi

if [ "$1" ]
then
    package=$1
    shift
	if [ "$1" ]
	then
		reponame=$1
		shift
	else
		if [ "megasync" == "$package" ]; then
			reponame=MEGAsync
		elif [ "nautilus-megasync" == "$package" ]; then
			reponame=MEGAShellExtNautilus
		else
			reponame=$package
		fi
	fi
else
	package=megasync
	reponame=MEGAsync
fi


if [ ! -d $DLfolder ]; then
  echo "downloading current repositories to $DLfolder"
  (mkdir $DLfolder; cd $DLfolder; sudo wget -r --no-parent --reject "index.html*" https://mega.nz/linux/MEGAsync/)
fi

DLFinalFolder=$DLfolder/mega.nz/linux/MEGAsync

for repo in CentOS_7 Fedora_19 Fedora_20 Fedora_21 Fedora_22 Fedora_23 Fedora_24 Fedora_25 openSUSE_12.2 openSUSE_12.3 openSUSE_13.1 openSUSE_13.2 openSUSE_Leap_42.1 openSUSE_Leap_42.2 openSUSE_Tumbleweed RHEL_7 ScientificLinux_7; do
	for arch in i586 x86_64; do 
	
		if [ "$arch" == "i586" ] ; then 
			if [ "$repo" == "CentOS_7" ] || [ "$repo" == "ScientificLinux_7" ] || [ "$repo" == "RHEL_7" ] ; then 
				continue
			fi
		fi
			
		archweb=$arch
		if [ ! -d $DLFinalFolder/$repo/$archweb ]; then
			archweb=${arch/i586/i686}; 
		fi
		
		echo "REPO=$repo ARCH=$arch ARCHWEB=$archweb"
		if [ ! -d $DLFinalFolder/$repo/$archweb ]; then
			echo "   COULD NOT FIND DATA in DL folder: $DLFinalFolder/$repo/$archweb, doing nothing"
			continue
		fi

		rm $obsbuildpath/RPM/$repo/$arch/$reponame/$package*
		for i in $DLFinalFolder/$repo/$archweb/$package*rpm $DLFinalFolder/$repo/src/$package*src.rpm; do 
			DEST=`basename $i`; 
			cp $i $obsbuildpath/RPM/$repo/$arch/$reponame/; 
		done; 
		chown obsrun:obsrun $obsbuildpath/RPM/$repo/$arch/$reponame/*;
		
		echo "--------------------------------"
	done
	
done

for repo in Arch_Extra Debian_7.0 Debian_8.0 xUbuntu_12.04 xUbuntu_12.10 xUbuntu_13.04 xUbuntu_13.10 xUbuntu_14.04 xUbuntu_14.10 xUbuntu_15.04 xUbuntu_15.10 xUbuntu_16.04 xUbuntu_16.10; do
	for arch in i586 x86_64; do 
		
		if [ "$repo" == "Arch_Extra" ] ; then 
			archweb=${arch/i586/i686}; 
		else
			archweb=$(echo ${arch/x86_64/amd64} | sed "s#i586#i386#g")
		fi
		
		echo "REPO=$repo ARCH=$arch ARCHWEB=$archweb"
		if [ ! -d $DLFinalFolder/$repo/$archweb ]; then
			echo "   COULD NOT FIND DATA in DL folder: $DLFinalFolder/$repo/$archweb, doing nothing"
			continue
		fi
		
		if [ "$repo" == "Arch_Extra" ]; then
			rm $obsbuildpath/DEB/$repo/$arch/$reponame/$package*

			for i in $DLFinalFolder/$repo/$archweb/$package*tar.xz; do 
				DEST=`basename $i`; 
				cp $i $obsbuildpath/DEB/$repo/$arch/$reponame/; 
				cp ${i/$DEST/DEB_Arch_Extra.db.sig} $obsbuildpath/DEB/$repo/$arch/$reponame/${DEST}.sig; 
			done; 
			chown obsrun:obsrun $obsbuildpath/DEB/$repo/$arch/$reponame/*;
		else
			rm $obsbuildpath/DEB/$repo/$arch/$reponame/$package*
			for i in $DLFinalFolder/$repo/{*dsc,*tar.gz,$archweb/*changelog,$archweb/$package*$archweb.deb}; do 
				DEST=`basename $i`; 
				cp $i $obsbuildpath/DEB/$repo/$arch/$reponame/${DEST/changelog/changes}; 
			done; 
			chown obsrun:obsrun $obsbuildpath/DEB/$repo/$arch/$reponame/*;
			#changelog copy:
			cp $DLFinalFolder/$repo/$archweb/$package*changelog $obsrepospath/DEB/$repo/$archweb/
			chlog=$chlog"cp $DLFinalFolder/$repo/$archweb/$package*changelog $obsrepospath/DEB/$repo/$archweb/"$'\n'
		fi
		echo "--------------------------------"
	done
done


echo "get the changelogs and save them in the correct location:"
echo "$chlog "
echo "chown obsrun:obsrun $obsrepospath/DEB/*/*/*changelog"
