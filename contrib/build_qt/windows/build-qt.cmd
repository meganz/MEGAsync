SET MEGA_QT_MAJ_VER=5
SET MEGA_QT_MIN_VER=15
SET MEGA_QT_DOT_VER=11
SET MEGA_QT_VER=%MEGA_QT_MAJ_VER%.%MEGA_QT_MIN_VER%.%MEGA_QT_DOT_VER%

SET MEGA_PATCHES_DIR=%CD%\..\patches
SET MEGA_WORK_DIR=C:\Qt-build\%MEGA_QT_VER%

mkdir %MEGA_WORK_DIR%
cd %MEGA_WORK_DIR%

git clone git://code.qt.io/qt/qt5.git Src
cd Src
git checkout v%MEGA_QT_VER%-lts-lgpl
perl init-repository --module-subset=essential,qtwinextras,qtimageformats,qtquickcontrols,qtsvg,qtgraphicaleffects,qtdeclarative,qtquickcontrols2

FOR /D %%M in (%MEGA_PATCHES_DIR%\%MEGA_QT_VER%\*) DO (
	FOR %%I IN (%%M\*) DO (
		git apply -v --directory=%%~nM --ignore-whitespace %%I
	)
)

SET _ROOT=%CD%
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET _ROOT=

cd ..
mkdir build-x64
cd build-x64
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
call ..\Src\configure -nomake examples -nomake tests -opensource -schannel -confirm-license -prefix %MEGA_WORK_DIR%\x64 -force-debug-info -separate-debug-info -qt-zlib -no-jasper -qt-libjpeg -qt-libpng -qt-freetype -qt-pcre -qt-harfbuzz
call jom
call jom install

cd ..
mkdir build-x86
cd build-x86
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86
call ..\Src\configure -nomake examples -nomake tests -opensource -schannel -confirm-license -prefix %MEGA_WORK_DIR%\x86 -force-debug-info -separate-debug-info -qt-zlib -no-jasper -qt-libjpeg -qt-libpng -qt-freetype -qt-pcre -qt-harfbuzz
call jom
call jom install
