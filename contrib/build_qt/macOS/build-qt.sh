#!/bin/zsh -e

MEGA_QT_MAJ_VER=5
MEGA_QT_MIN_VER=15
MEGA_QT_DOT_VER=11
MEGA_QT_VER=$MEGA_QT_MAJ_VER.$MEGA_QT_MIN_VER.$MEGA_QT_DOT_VER

MEGA_PATCHES_DIR="${PWD}/../patches/${MEGA_QT_VER}"
MEGA_WORK_DIR="${HOME}/Qt-build/${MEGA_QT_VER}"
MEGA_QT_SOURCES_DIR="${MEGA_QT_VER}-sources"
MEGA_QT_BUILD_DIR="${MEGA_QT_VER}-build"

mkdir -p $MEGA_WORK_DIR
cd $MEGA_WORK_DIR

mkdir -p "${MEGA_QT_BUILD_DIR}-arm64"
mkdir -p "${MEGA_QT_BUILD_DIR}-x86_64"
mkdir -p $MEGA_QT_VER
mkdir -p $MEGA_QT_SOURCES_DIR

git clone git://code.qt.io/qt/qt5.git $MEGA_QT_SOURCES_DIR
cd $MEGA_QT_SOURCES_DIR
git checkout v${MEGA_QT_VER}-lts-lgpl
perl init-repository

for patch_directory in ${MEGA_PATCHES_DIR}/*; do
    for file in ${patch_directory}/*; do
		echo "Applying patch ${file}" 
		git apply -v --directory=`basename "$patch_directory"` --ignore-whitespace $file
    done
done

cd "../${MEGA_QT_BUILD_DIR}-arm64"
../${MEGA_QT_SOURCES_DIR}/configure QMAKE_APPLE_DEVICE_ARCHS=arm64 --prefix="${PWD}/../${MEGA_QT_VER}/arm64" -opensource -confirm-license -nomake examples -nomake tests -skip qtwebview -skip qtwebengine -skip qtwebchannel -skip qtconnectivity -skip qt3d -skip qtlocation -skip qtvirtualkeyboard  -force-debug-info -separate-debug-info -debug-and-release
make -j`sysctl -n hw.ncpu`
make install

cd "../${MEGA_QT_BUILD_DIR}-x86_64"
../${MEGA_QT_SOURCES_DIR}/configure QMAKE_APPLE_DEVICE_ARCHS=x86_64 --prefix="${PWD}/../${MEGA_QT_VER}/x86_64" -opensource -confirm-license -nomake examples -nomake tests -skip qtwebview -skip qtwebengine -skip qtwebchannel -skip qtconnectivity -skip qt3d -skip qtlocation -skip qtvirtualkeyboard  -force-debug-info -separate-debug-info -debug-and-release
make -j`sysctl -n hw.ncpu`
make install
