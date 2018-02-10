#!/bin/sh -e

Usage () {
    echo "Usage: installer_mac.sh [[--sign] | [--create-dmg]]"
}

APP_NAME=MEGAsync
MOUNTDIR=tmp
RESOURCES=installer/resourcesDMG
QTBASE=/QT/qt5/qtbase

AVCODEC_VERSION=libavcodec.57.dylib
AVFORMAT_VERSION=libavformat.57.dylib
AVUTIL_VERSION=libavutil.55.dylib
SWSCALE_VERSION=libswscale.4.dylib

AVCODEC_PATH=src/MEGASync/mega/bindings/qt/3rdparty/libs/$AVCODEC_VERSION
AVFORMAT_PATH=src/MEGASync/mega/bindings/qt/3rdparty/libs/$AVFORMAT_VERSION
AVUTIL_PATH=src/MEGASync/mega/bindings/qt/3rdparty/libs/$AVUTIL_VERSION
SWSCALE_PATH=src/MEGASync/mega/bindings/qt/3rdparty/libs/$SWSCALE_VERSION


sign=0
createdmg=0

while [ "$1" != "" ]; do
    case $1 in
        --sign )		sign=1
                                ;;
        --create-dmg )		createdmg=1
                                ;;
        -h | --help )           Usage
                                exit
                                ;;
        * )                     Usage
                                exit 1
    esac
    shift
done

rm -rf Release_x64
mkdir Release_x64
cd Release_x64
$QTBASE/bin/lrelease ../src/MEGASync/MEGASync.pro
$QTBASE/bin/qmake -r ../src -spec macx-g++ CONFIG+=release CONFIG+=x86_64 -nocache
make -j4
cp -R MEGASync/MEGAsync.app MEGASync/MEGAsync_orig.app
$QTBASE/bin/macdeployqt MEGASync/MEGAsync.app
dsymutil MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync -o MEGAsync.app.dSYM
strip MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
dsymutil MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader -o MEGAloader.dSYM
strip MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader
dsymutil MEGAUpdater/MEGAupdater.app/Contents/MacOS/MEGAupdater -o MEGAupdater.dSYM
strip MEGAUpdater/MEGAupdater.app/Contents/MacOS/MEGAupdater
dsymutil MEGADeprecatedVersion/MEGADeprecatedVersion.app/Contents/MacOS/MEGADeprecatedVersion -o MEGADeprecatedVersion.dSYM
strip MEGADeprecatedVersion/MEGADeprecatedVersion.app/Contents/MacOS/MEGADeprecatedVersion
mv MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
mv MEGALoader/MEGAloader.app/Contents/MacOS/MEGAloader MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
mv MEGAUpdater/MEGAupdater.app/Contents/MacOS/MEGAupdater MEGASync/MEGAsync.app/Contents/MacOS/MEGAupdater
mv MEGADeprecatedVersion/MEGADeprecatedVersion.app/Contents/MacOS/MEGADeprecatedVersion MEGASync/MEGAsync.app/Contents/MacOS/MEGADeprecatedVersion

cp -L ../$AVCODEC_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
cp -L ../$AVFORMAT_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
cp -L ../$AVUTIL_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
cp -L ../$SWSCALE_PATH MEGASync/MEGAsync.app/Contents/Frameworks/

if [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVCODEC_VERSION ]  \
	|| [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVFORMAT_VERSION ]  \
	|| [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVUTIL_VERSION ]  \
	|| [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$SWSCALE_VERSION ];
then
	echo "Error copying FFmpeg libs to app bundle."
	exit 1
fi

install_name_tool -change @loader_path/$AVCODEC_VERSION @executable_path/../Frameworks/$AVCODEC_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
install_name_tool -change @loader_path/$AVFORMAT_VERSION @executable_path/../Frameworks/$AVFORMAT_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
install_name_tool -change @loader_path/$AVUTIL_VERSION @executable_path/../Frameworks/$AVUTIL_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient
install_name_tool -change @loader_path/$SWSCALE_VERSION @executable_path/../Frameworks/$SWSCALE_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient

otool -L MEGASync/MEGAsync.app/Contents/MacOS/MEGAclient

mv MEGAsync/MEGAsync.app ./

#Attach shell extension
xcodebuild clean build CODE_SIGN_IDENTITY="-" CODE_SIGNING_REQUIRED=NO -jobs "$(sysctl -n hw.ncpu)" -configuration Release -target MEGAShellExtFinder -project ../src/MEGAShellExtFinder/MEGAFinderSync.xcodeproj/
cp -a ../src/MEGAShellExtFinder/build/Release/MEGAShellExtFinder.appex $APP_NAME.app/Contents/Plugins/

if [ "$sign" = "1" ]; then
	cp -R $APP_NAME.app ${APP_NAME}_unsigned.app
	echo "Signing 'APPBUNDLE'"
	codesign --force --verify --verbose --preserve-metadata=entitlements --sign "Developer ID Application: Mega Limited" --deep $APP_NAME.app
	echo "Checking signature"
	spctl -vv -a $APP_NAME.app
fi

if [ "$createdmg" = "1" ]; then
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
	ln -s /Applications/ $MOUNTDIR/$APP_NAME/Applications

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
fi

echo "Cleaning"
rm -rf MEGAsync
rm -rf MEGALoader
rm -rf MEGAUpdater
rm -rf MEGADeprecatedVersion

echo "DONE"
