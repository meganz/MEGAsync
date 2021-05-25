#!/bin/bash

#use like this (in debian WSL):
#mattw@DESKTOP-13186O8:~$ dos2unix /mnt/c/dev/desktop/contrib/updater/gen_fileswinWSL.sh
#mattw@DESKTOP-13186O8:~$ /mnt/c/dev/desktop/contrib/updater/gen_fileswinWSL.sh /mnt/c/Users/mattw/AppData/Local/MEGAsync
#then diff with existing version and adjust

echo $1
(cd $1 && for i in $(find . -type f); do echo "$i;$i;$(sha256sum $i | awk '{print $1}')"; done | sed "s#_CodeSignature#VCodeSignature#g" | sed "s#\.\/##g" | sort -f |  sed "s#VCodeSignature#_CodeSignature#g")
