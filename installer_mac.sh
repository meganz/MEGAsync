#!/bin/sh

rm -rf Release_x64
mkdir Release_x64
cd Release_x64
qmake -r ../Source -spec macx-g++ CONFIG+=release CONFIG+=x86_64 -nocache
make
macdeployqt MEGASync/MEGAsync.app
dsymutil MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync -o MEGAsync.app.dSYM
strip MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
dsymutil MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader -o MEGAloader.dSYM
strip MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader
mv MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
mv MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
mv MEGASync/MEGAsync.app ./

