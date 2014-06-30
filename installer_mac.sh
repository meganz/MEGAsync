#!/bin/sh

APP_NAME=MEGAsync
MOUNTDIR=tmp
RESOURCES=resourcesDMG

rm -rf Release_x64
mkdir Release_x64
cd Release_x64
~/Qt5.3.0/5.3/clang_64/bin/lrelease ../Source/MEGASync/MEGASync.pro
~/Qt5.3.0/5.3/clang_64/bin/qmake -r ../Source -spec macx-g++ CONFIG+=release CONFIG+=x86_64 -nocache
make
~/Qt5.3.0/5.3/clang_64/bin/macdeployqt MEGASync/MEGAsync.app
dsymutil MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync -o MEGAsync.app.dSYM
strip MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
dsymutil MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader -o MEGAloader.dSYM
strip MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader
mv MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
mv MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
mv MEGASync/MEGAsync.app ./



echo "DMG CREATION PROCESS..."
echo "Creating temporary Disk Image (1/7)"
#Create a temporary Disk Image
/usr/bin/hdiutil create -srcfolder $APP_NAME.app/ -volname $APP_NAME -ov $APP_NAME-tmp.dmg -format UDRW >/dev/null

echo "Attaching the temporary image (2/7)"
#Attach the temporary image
mkdir $MOUNTDIR
/usr/bin/hdiutil attach $APP_NAME-tmp.dmg -mountroot $MOUNTDIR >/dev/null

echo "Copying resources (3/7)" 
#Copy the background, the volume icon and DS_Store files
unzip -d $MOUNTDIR/$APP_NAME ../$RESOURCES.zip
/usr/bin/SetFile -a C $MOUNTDIR/$APP_NAME

echo "Adding symlinks (4/7)"
#Add a symbolic link to the Applications directory
ln -s /Applications/ "$MOUNTDIR/$APP_NAME/ " 

echo "Detaching temporary Disk Image (5/7)"
#Detach the temporary image
/usr/bin/hdiutil detach $MOUNTDIR/$APP_NAME >/dev/null

echo "Compressing Image (6/7)"
#Compress it to a new image
/usr/bin/hdiutil convert $APP_NAME-tmp.dmg -format UDZO -o $APP_NAME.dmg >/dev/null

echo "Deleting temporary image (7/7)"
#Delete the temporary image
rm $APP_NAME-tmp.dmg
rmdir $MOUNTDIR

echo "DONE"