#!/bin/bash
echo $1
(cd $1 && for i in $(find . -type f); do echo "$i;$i;$(sha256sum $i | awk '{print $1}')"; done | sed "s#_CodeSignature#VCodeSignature#g" | sed "s#\.\/##g" | sort -f |  sed "s#VCodeSignature#_CodeSignature#g")
