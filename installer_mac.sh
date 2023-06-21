#!/bin/zsh -e

Usage () {
    echo "Usage: installer_mac.sh [[--arch [arm64|x86_64]] [--build | --build-cmake] | [--sign] | [--create-dmg] | [--notarize] | [--full-pkg | --full-pkg-cmake]]"
    echo "    --arch [arm64|x86_64]  : Arch target. It will build for the host arch if not defined."
    echo "    --build                : Builds the app and creates the bundle using qmake."
    echo "    --build-cmake          : Idem but using cmake"
    echo "    --sign                 : Sign the app"
    echo "    --create-dmg           : Create the dmg package"
    echo "    --notarize             : Notarize package against Apple systems."
    echo "    --full-pkg             : Implies and overrides all the above using qmake"
    echo "    --full-pkg-cmake       : Idem but using cmake"
    echo ""
    echo "Environment variables needed to build:"
    echo "    MEGAQTPATH : Point it to a valid Qt installation path"
    echo "    VCPKGPATH : Point it to a directory containing a valid vcpkg installation"
    echo ""
    echo "Note: --build and --build-cmake are mutually exclusive."
    echo "      --full-pkg and --full-pkg-cmake are mutually exclusive."
    echo ""
}

if [ $# -eq 0 ]; then
   Usage
   exit 1
fi

APP_NAME=MEGAsync
ID_BUNDLE=mega.mac
MOUNTDIR=tmp
RESOURCES=installer/resourcesDMG
MSYNC_PREFIX=MEGASync/
MUPDATER_PREFIX=MEGAUpdater/

host_arch=`uname -m`
target_arch=${host_arch}
full_pkg=0
full_pkg_cmake=0
build=0
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
        --build )
            build=1
            if [ ${build_cmake} -eq 1 ]; then Usage; echo "Error: --build and --build-cmake are mutually exclusive."; exit 1; fi
            ;;
        --build-cmake )
            build_cmake=1
            if [ ${build} -eq 1 ]; then Usage; echo "Error: --build and --build-cmake are mutually exclusive."; exit 1; fi
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
        --full-pkg )
            if [ ${full_pkg_cmake} -eq 1 ]; then Usage; echo "Error: --full-pkg and --full-pkg-cmake are mutually exclusive."; exit 1; fi
            full_pkg=1
            ;;
        --full-pkg-cmake )
            if [ ${full_pkg} -eq 1 ]; then Usage; echo "Error: --full-pkg and --full-pkg-cmake are mutually exclusive."; exit 1; fi
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

if [ ${full_pkg} -eq 1 ]; then
    build=1
    build_cmake=0
    sign=1
    createdmg=1
    notarize=1
fi

if [ ${full_pkg_cmake} -eq 1 ]; then
    build=0
    build_cmake=1
    sign=1
    createdmg=1
    notarize=1
fi

if [ ${build} -ne 1 -a ${build_cmake} -ne 1 -a ${sign} -ne 1 -a ${createdmg} -ne 1 -a ${notarize} -ne 1 ]; then
   Usage
   echo "Error: No action selected. Nothing to do."
   exit 1
fi

if [ ${build} -eq 1 -o ${build_cmake} -eq 1 ]; then
    build_time_start=`date +%s`

    if [ -z "${MEGAQTPATH}" ] || [ ! -d "${MEGAQTPATH}/bin" ]; then
        echo "Please set MEGAQTPATH env variable to a valid QT installation path!"
        exit 1;
    fi
    if [ -z "${VCPKGPATH}" ] || [ ! -d "${VCPKGPATH}/vcpkg/installed" ]; then
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

    if [ ${build_cmake} -ne 1 ]; then
        AVCODEC_VERSION=libavcodec.58.dylib
        AVFORMAT_VERSION=libavformat.58.dylib
        AVUTIL_VERSION=libavutil.56.dylib
        SWSCALE_VERSION=libswscale.5.dylib
        CARES_VERSION=libcares.2.dylib
        CURL_VERSION=libcurl.dylib

        AVCODEC_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$AVCODEC_VERSION
        AVFORMAT_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$AVFORMAT_VERSION
        AVUTIL_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$AVUTIL_VERSION
        SWSCALE_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$SWSCALE_VERSION
        CARES_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$CARES_VERSION
        CURL_PATH=${VCPKGPATH}/vcpkg/installed/${target_arch//x86_64/x64}-osx-mega/lib/$CURL_VERSION
    fi

    # Clean previous build
    rm -rf Release_${target_arch}
    mkdir Release_${target_arch}
    cd Release_${target_arch}

    # Build binaries
    if [ ${build_cmake} -eq 1 ]; then
        # Detect crosscompilation and set CMAKE_OSX_ARCHITECTURES.
        if  [ "${target_arch}" != "${host_arch}" ]; then
            CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=${target_arch}"
        fi

        cmake -DUSE_THIRDPARTY_FROM_VCPKG=1 -DMega3rdPartyDir=${VCPKGPATH} -DCMAKE_PREFIX_PATH=${MEGAQTPATH} -DCMAKE_BUILD_TYPE=RelWithDebInfo ${CMAKE_EXTRA} -S ../contrib/cmake
        cmake --build ./ --target MEGAsync -j`sysctl -n hw.ncpu`
        cmake --build ./ --target MEGAupdater -j`sysctl -n hw.ncpu`
        MSYNC_PREFIX=""
        MUPDATER_PREFIX=""
    else
        # crosscompilation detection should be managed detecting the qmake taget and host arch in the project files.
        cp ../src/MEGASync/mega/contrib/official_build_configs/macos/config.h ../src/MEGASync/mega/include/mega/config.h
        ${MEGAQTPATH}/bin/lrelease ../src/MEGASync/MEGASync.pro
        ${MEGAQTPATH}/bin/qmake "CONFIG += FULLREQUIREMENTS" "THIRDPARTY_VCPKG_BASE_PATH=${VCPKGPATH}" -r ../src -spec macx-clang CONFIG+=release -nocache
        make -j`sysctl -n hw.ncpu`
    fi

    # Prepare bundle
    cp -R ${MSYNC_PREFIX}MEGAsync.app ${MSYNC_PREFIX}MEGAsync_orig.app
    ${MEGAQTPATH}/bin/macdeployqt ${MSYNC_PREFIX}MEGAsync.app -no-strip
    dsymutil ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAsync -o MEGAsync.app.dSYM
    strip ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAsync
    dsymutil ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater -o MEGAupdater.dSYM
    strip ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater

    mv ${MUPDATER_PREFIX}MEGAupdater.app/Contents/MacOS/MEGAupdater ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAupdater

    touch ${MSYNC_PREFIX}MEGAsync.app/Contents/MacOS/MEGAclient

    if [ ${build_cmake} -ne 1 ]; then
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVCODEC_VERSION ] && cp -L $AVCODEC_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVFORMAT_VERSION ] && cp -L $AVFORMAT_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVUTIL_VERSION ] && cp -L $AVUTIL_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$SWSCALE_VERSION ] && cp -L $SWSCALE_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$CARES_VERSION ] && cp -L $CARES_PATH MEGASync/MEGAsync.app/Contents/Frameworks/
        [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$CURL_VERSION ] && cp -L $CURL_PATH MEGASync/MEGAsync.app/Contents/Frameworks/

        if [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVCODEC_VERSION ]  \
            || [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVFORMAT_VERSION ]  \
            || [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$AVUTIL_VERSION ]  \
            || [ ! -f MEGASync/MEGAsync.app/Contents/Frameworks/$SWSCALE_VERSION ];
        then
            echo "Error copying FFmpeg libs to app bundle."
            exit 1
        fi
    fi

    MEGASYNC_VERSION=`grep -o -E '#define VER_PRODUCTVERSION_STR\s+(.*)' ../src/MEGASync/control/Version.h | grep -oE '\d+\.\d+\.\d+'`
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $MEGASYNC_VERSION" "${MSYNC_PREFIX}$APP_NAME.app/Contents/Info.plist"

    if [ ${build_cmake} -ne 1 ]; then
        install_name_tool -change @loader_path/$AVCODEC_VERSION @executable_path/../Frameworks/$AVCODEC_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
        install_name_tool -change @loader_path/$AVFORMAT_VERSION @executable_path/../Frameworks/$AVFORMAT_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
        install_name_tool -change @loader_path/$AVUTIL_VERSION @executable_path/../Frameworks/$AVUTIL_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync
        install_name_tool -change @loader_path/$SWSCALE_VERSION @executable_path/../Frameworks/$SWSCALE_VERSION MEGASync/MEGAsync.app/Contents/MacOS/MEGAsync

        rm -r $APP_NAME.app || :
        mv $MSYNC_PREFIX/$APP_NAME.app ./
    fi

    otool -L MEGAsync.app/Contents/MacOS/MEGAsync

    #Attach shell extension
    xcodebuild clean build CODE_SIGN_IDENTITY="-" CODE_SIGNING_REQUIRED=NO -jobs "$(sysctl -n hw.ncpu)" -configuration Release -target MEGAShellExtFinder -project ../src/MEGAShellExtFinder/MEGAFinderSync.xcodeproj/
    cp -a ../src/MEGAShellExtFinder/build/Release/MEGAShellExtFinder.appex $APP_NAME.app/Contents/Plugins/
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
	/usr/bin/hdiutil create -srcfolder $APP_NAME.app/ -volname $APP_NAME -ov $APP_NAME-tmp.dmg -fs HFS+ -format UDRW >/dev/null

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
    spctl --assess -vv -a $MOUNTDIR/$APP_NAME/$APP_NAME.app
    hdiutil detach $MOUNTDIR/$APP_NAME >/dev/null
    rmdir $MOUNTDIR

	cd ..
    notarize_time=`expr $(date +%s) - $notarize_time_start`
fi

echo ""
if [ ${build} -eq 1 -o ${build_cmake} -eq 1 ]; then echo "Build:        ${build_time} s"; fi
if [ ${sign} -eq 1 ]; then echo "Sign:         ${sign_time} s"; fi
if [ ${createdmg} -eq 1 ]; then echo "dmg:          ${dmg_time} s"; fi
if [ ${notarize} -eq 1 ]; then echo "Notarization: ${notarize_time} s"; fi
echo ""
echo "DONE in       "`expr ${build_time} + ${sign_time} + ${dmg_time} + ${notarize_time}`" s"
