#!/bin/zsh -e

Usage () {
    echo "Usage: installer_mac.sh [[--arch [arm64|x86_64]] [--build-cmake] | [--sign] | [--create-dmg] | [--notarize] | [--full-pkg-cmake]]"
    echo "    --arch [arm64|x86_64]  : Arch target. It will build for the host arch if not defined."
    echo "    --build-cmake          : Idem but using cmake"
    echo "    --sign                 : Sign the app"
    echo "    --create-dmg           : Create the dmg package"
    echo "    --notarize             : Notarize package against Apple systems."
    echo "    --full-pkg-cmake       : Idem but using cmake"
    echo ""
    echo "Environment variables needed to build:"
    echo "    MEGAQTPATH : Point it to a valid Qt installation path"
    echo "    VCPKGPATH : Point it to a directory containing a valid vcpkg installation"
    echo ""
}

if [ $# -eq 0 ]; then
   Usage
   exit 1
fi

APP_NAME=MEGAsync
VOLUME_NAME="Install MEGA"
ID_BUNDLE=mega.mac
MOUNTDIR=tmp
RESOURCES=installer/macOS/resourcesDMG
MSYNC_PREFIX="src/MEGASync/"
MUPDATER_PREFIX="src/MEGAUpdater/"

host_arch=`uname -m`
target_arch=${host_arch}
full_pkg_cmake=0
build_cmake=0
sign=0
createdmg=0
notarize=0

build_time=0
sign_time=0
dmg_time=0
notarize_time=0
total_time=0


while [ "$1" != "" ]; do
    case $1 in
        --arch )
            shift
            target_arch="${1}"
            if [ "${target_arch}" != "arm64" ] && [ "${target_arch}" != "x86_64" ]; then Usage; echo "Error: Invalid arch value."; exit 1; fi
            ;;
        --build-cmake )
            build_cmake=1
            ;;
        --sign )
            sign=1
            ;;
        --create-dmg )
            createdmg=1
            ;;
        --notarize )
            notarize=1
            ;;
        --full-pkg-cmake )
            full_pkg_cmake=1
            ;;
        -h | --help )
            Usage
            exit
            ;;
        * )
            Usage
            echo "Unknown parameter: ${1}"
            exit 1
    esac
    shift
done

if [ ${full_pkg_cmake} -eq 1 ]; then
    build_cmake=1
    sign=1
    createdmg=1
    notarize=1
fi

if [ ${build_cmake} -ne 1 -a ${sign} -ne 1 -a ${createdmg} -ne 1 -a ${notarize} -ne 1 ]; then
   Usage
   echo "Error: No action selected. Nothing to do."
   exit 1
fi

if [ ${build_cmake} -eq 1 ]; then
    build_time_start=`date +%s`

    if [ -z "${MEGAQTPATH}" ] || [ ! -d "${MEGAQTPATH}/bin" ]; then
        echo "Please set MEGAQTPATH env variable to a valid QT installation path!"
        exit 1;
    fi
    if [ -z "${VCPKGPATH}" ]; then
        echo "Please set VCPKGPATH env variable to a directory containing a valid vcpkg installation!"
        exit 1;
    fi

    MEGAQTPATH="$(cd "$MEGAQTPATH" && pwd -P)"
    echo "Building with:"
    echo "  MEGAQTPATH : ${MEGAQTPATH}"
    echo "  VCPKGPATH  : ${VCPKGPATH}"

    if ! lipo -archs ${MEGAQTPATH}/lib/QtCore.framework/QtCore 2>/dev/null | grep ${target_arch} >/dev/null; then
        echo "Qt libs for ${target_arch} are not present in ${MEGAQTPATH} Qt installation."
        exit 1
    fi

    # Clean previous build
    rm -rf Release_${target_arch}
    mkdir Release_${target_arch}
    cd Release_${target_arch}

    # Build binaries

    # Detect crosscompilation and set CMAKE_OSX_ARCHITECTURES.
    if  [ "${target_arch}" != "${host_arch}" ]; then
        CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=${target_arch}"
    fi

    cmake -DVCPKG_ROOT=${VCPKGPATH} -DCMAKE_PREFIX_PATH=${MEGAQTPATH} -DCMAKE_BUILD_TYPE=RelWithDebInfo ${CMAKE_EXTRA} -S ../
    cmake --build ./ --target MEGAsync -j`sysctl -n hw.ncpu`
    cmake --build ./ --target MEGAupdater -j`sysctl -n hw.ncpu`

    # Prepare bundle
    cp -R ${MSYNC_PREFIX}MEGAsync.app ${MSYNC_PREFIX}MEGAsync_orig.app
    ${MEGAQTPATH}/bin/macdeployqt ${MSYNC_PREFIX}MEGAsync.app -qmldir=../src/MEGASync/gui/qml -no-strip
    dsymutil ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAsync -o ${MSYNC_PREFIX}MEGAsync.app.dSYM 
    zip -r ${MSYNC_PREFIX}MEGAsync.app.dSYM.zip ${MSYNC_PREFIX}MEGAsync.app.dSYM
    rm -rf ${MSYNC_PREFIX}MEGAsync.app.dSYM
    strip ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAsync
    dsymutil ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater -o ${MUPDATER_PREFIX}MEGAupdater.dSYM 
    zip -r ${MUPDATER_PREFIX}MEGAupdater.app.dSYM.zip ${MUPDATER_PREFIX}MEGAupdater.dSYM 
    rm -rf ${MUPDATER_PREFIX}MEGAupdater.dSYM
    strip ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater

    mv ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAupdater

    touch ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAclient

    # Delete unused debug libs
    find ${MSYNC_PREFIX}MEGAsync.app/Contents -type d -name "*.dSYM" -exec rm -r {} +
    # need to remove .prl leftovers from frameworks after macdeployqt
    find ${MSYNC_PREFIX}MEGAsync.app/Contents -type f -name "*.prl" -exec rm -f {} +

    # Generate symlinks file to recreate after update
    MEGASYNC_VERSION_CODE=`grep -o -E '#define VER_FILEVERSION_CODE\s+(.*)' ../src/MEGASync/control/Version.h | grep -oE '\d+'`
    echo  ${MEGASYNC_VERSION_CODE} > ${MSYNC_PREFIX}MEGAsync.app/Contents/Resources/mega.links

    pushd .
    cd ${MSYNC_PREFIX}MEGAsync.app
    find . -type l -exec bash -c 'echo $(readlink "$0") ; echo "$0";' {} \; >> ./Contents/Resources/mega.links
    popd
 
    MEGASYNC_VERSION=`grep -o -E '#define VER_PRODUCTVERSION_STR\s+(.*)' ../src/MEGASync/control/Version.h | grep -oE '\d+\.\d+\.\d+'`
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $MEGASYNC_VERSION" "${MSYNC_PREFIX}$APP_NAME.app/Contents/Info.plist"

    otool -L ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAsync

    #Attach shell extension
    xcodebuild clean build CODE_SIGN_IDENTITY="-" CODE_SIGNING_REQUIRED=NO -jobs "$(sysctl -n hw.ncpu)" -configuration Release -target MEGAShellExtFinder -project ../src/MEGAShellExtFinder/MEGAFinderSync.xcodeproj/
    cp -a ../src/MEGAShellExtFinder/build/Release/MEGAShellExtFinder.appex ${MSYNC_PREFIX}$APP_NAME.app/Contents/Plugins/
    cd ..

    build_time=`expr $(date +%s) - $build_time_start`
fi

if [ "$sign" = "1" ]; then
    sign_time_start=`date +%s`
	cd Release_${target_arch}
	cp -R $APP_NAME.app ${APP_NAME}_unsigned.app
	echo "Signing 'APPBUNDLE'"
	codesign --force --verify --verbose --preserve-metadata=entitlements --options runtime --sign "Developer ID Application: Mega Limited" --deep $APP_NAME.app
	echo "Checking signature"
	spctl -vv -a $APP_NAME.app
	cd ..
    sign_time=`expr $(date +%s) - $sign_time_start`
fi

if [ "$createdmg" = "1" ]; then
    dmg_time_start=`date +%s`
	cd Release_${target_arch}
	[ -f $APP_NAME.dmg ] && rm $APP_NAME.dmg
	echo "DMG CREATION PROCESS..."
	echo "Creating temporary Disk Image (1/7)"
	#Create a temporary Disk Image
	/usr/bin/hdiutil create -srcfolder ${MSYNC_PREFIX}$APP_NAME.app/ -volname $VOLUME_NAME -ov ${MSYNC_PREFIX}$APP_NAME-tmp.dmg -fs HFS+ -format UDRW >/dev/null

	echo "Attaching the temporary image (2/7)"
	#Attach the temporary image
	mkdir $MOUNTDIR
	/usr/bin/hdiutil attach ${MSYNC_PREFIX}$APP_NAME-tmp.dmg -mountroot $MOUNTDIR >/dev/null

	echo "Copying resources (3/7)"
	#Copy the background, the volume icon and DS_Store files
	unzip -d $MOUNTDIR/$VOLUME_NAME ../$RESOURCES.zip
	/usr/bin/SetFile -a C $MOUNTDIR/$VOLUME_NAME

	echo "Adding symlinks (4/7)"
	#Add a symbolic link to the Applications directory
	ln -s /Applications/ $MOUNTDIR/$VOLUME_NAME/Applications

    # Delete unnecessary file system events log if possible
    echo "Deleting .fseventsd"
    rm -rf $MOUNTDIR/$VOLUME_NAME/.fseventsd || true

	echo "Detaching temporary Disk Image (5/7)"
	#Detach the temporary image
	/usr/bin/hdiutil detach $MOUNTDIR/$VOLUME_NAME >/dev/null

	echo "Compressing Image (6/7)"
	#Compress it to a new image
	/usr/bin/hdiutil convert ${MSYNC_PREFIX}$APP_NAME-tmp.dmg -format UDZO -o ${MSYNC_PREFIX}$APP_NAME.dmg >/dev/null

	echo "Deleting temporary image (7/7)"
	#Delete the temporary image
	rm ${MSYNC_PREFIX}$APP_NAME-tmp.dmg
	rmdir $MOUNTDIR
	cd ..
    dmg_time=`expr $(date +%s) - $dmg_time_start`
fi

if [ "$notarize" = "1" ]; then
    notarize_time_start=`date +%s`
	cd Release_${target_arch}
	if [ ! -f $APP_NAME.dmg ];then
		echo ""
		echo "There is no dmg to be notarized."
		echo ""
		exit 1
	fi

	echo "Sending dmg for notarization (1/3)"

	xcrun notarytool submit $APP_NAME.dmg  --keychain-profile "AC_PASSWORD" --wait 2>&1 | tee notarylog.txt
    echo >> notarylog.txt

	xcrun stapler staple -v $APP_NAME.dmg 2>&1 | tee -a notarylog.txt
    
    echo "Stapling ok (2/3)"

    #Mount dmg volume to check if app bundle is notarized
    echo "Checking signature and notarization (3/3)"
    mkdir $MOUNTDIR || :
    hdiutil attach $APP_NAME.dmg -mountroot $MOUNTDIR >/dev/null
    spctl --assess -vv -a $MOUNTDIR/$VOLUME_NAME/$APP_NAME.app
    hdiutil detach $MOUNTDIR/$VOLUME_NAME >/dev/null
    rmdir $MOUNTDIR

	cd ..
    notarize_time=`expr $(date +%s) - $notarize_time_start`
fi

echo ""
if [ ${build_cmake} -eq 1 ]; then echo "Build:        ${build_time} s"; fi
if [ ${sign} -eq 1 ]; then echo "Sign:         ${sign_time} s"; fi
if [ ${createdmg} -eq 1 ]; then echo "dmg:          ${dmg_time} s"; fi
if [ ${notarize} -eq 1 ]; then echo "Notarization: ${notarize_time} s"; fi
echo ""
echo "DONE in       "`expr ${build_time} + ${sign_time} + ${dmg_time} + ${notarize_time}`" s"
