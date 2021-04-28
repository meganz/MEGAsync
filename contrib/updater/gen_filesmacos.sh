#!/bin/zsh -e

(cd /Applications/MEGAsync.app && for i in $(find Contents -type f); do echo "$i;$i;$(shasum -a 256 $i | awk '{print $1}')"; done | sed "s#_CodeSignature#VCodeSignature#g" | sort -f |  sed "s#VCodeSignature#_CodeSignature#g" | grep -v ";Contents/MacOS/MEGAsync;")
