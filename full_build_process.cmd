@echo off 

echo %QT_DIR%

IF "%1%" EQU "-help" (
	goto Usage
)

SET MEGA_ARCH=32/64
SET MEGA_SKIP_32_BIT_BUILD=false
SET MEGA_CORES=0
SET MEGA_VERSION_SUFFIX=

IF NOT "%1" == "" (
	SET MEGA_ARCH=%1
	SET MEGA_CORES=%2
	SET MEGA_VERSION_SUFFIX=%3
	
	IF [%MEGA_VCPKGPATH%]==[] (
		echo "Error: MEGA_VCPKGPATH environment variable is not set. Please set it."
		goto Usage
	)
	
	:: CHECK NUMBER OF ARGUMENTS
	IF "%2" == "" (
		echo "Error: too few arguments"
		goto Usage
	)
	IF NOT "%4" == "" (
		echo "Error: too many arguments"
		goto Usage
	)
) ELSE (
	IF [%MEGA_VCPKGPATH%]==[] (
		SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty\vcpkg
	)
)

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.16\x64
	)
)

IF [%MEGA_WIN_KITVER%]==[] (
	SET MEGA_WIN_KITVER=10.0.19041.0
)


:: CHECK ARCHITECTURE
IF "%MEGA_ARCH%" EQU "64" (
	echo "Info: Building x64 only"
	SET MEGA_SKIP_32_BIT_BUILD=true
) ELSE (
	IF "%MEGA_ARCH%" EQU "32/64" (
		echo "Info: Building both x64 and x86"
	) ELSE (
		echo "Please add the architecture as first parameter: 64 or 32/64"
		goto Usage
	)
)

:: CHECK CORES
SET "VALID_CORES=1"
IF %MEGA_CORES% LSS 0 (
	SET "VALID_CORES=0"
)
IF %MEGA_CORES% GTR 16 (
	SET "VALID_CORES=0"
)
IF %MEGA_CORES% EQU 0 (
	FOR /f "tokens=2 delims==" %%f IN ('wmic cpu get NumberOfLogicalProcessors /value ^| find "="') DO SET MEGA_CORES=%%f
)
IF %VALID_CORES% EQU 0 (
	echo "Please add a correct core argument: 1 to 16, or 0 for default value"
	goto Usage
)

echo "Info: CORES SET to %MEGA_CORES%"

REM Clean up any previous leftovers
IF EXIST built32 (
    rmdir /s /q built32
)
IF EXIST built64 (
    rmdir /s /q built64
)
IF EXIST build-x64-windows-mega (
    rmdir /s /q build-x64-windows-mega
)
IF EXIST build-x86-windows-mega (
    rmdir /s /q build-x86-windows-mega
)

REM Build installers
call production_build.cmd  || exit 1 /b
call sign_products.cmd  || exit 1 /b
call deploy_qt.cmd  || exit 1 /b
call gather_built_products.cmd  || exit 1 /b
call make_uninstallers.cmd  || exit 1 /b
call make_installers.cmd  || exit 1 /b

exit /B

:Usage
echo "Usage: %~0 [-help] [64|32/64 <cores number> [<suffix>]]"
echo Script building, signing and creating the installer(s)
echo It can take 0, 1, 2 or 3 arguments:
echo 	- -help: this message
echo 	- 0 arguments: use these settings: 32/64 1
echo 	- Architecture : 64 or 32/64 to build either for 64 bit or both 32 and 64 bit
echo 	- Cores: the number of cores to build the project, or 0 for default value (number of logical cores on the machine)
echo 	- Suffix for installer: The installer will add this suffix to the version. [OPTIONAl]
echo MEGA_VCPKGPATH environment variable should be set to the root of the 3rd party dir.
echo MEGA_QTPATH environment variable should be set to the Qt install dir. Takes the value of MEGAQTPATH, or defaults to C:\Qt\5.15.16\x64
echo MEGA_WIN_KITVER environment variable can be used to set the Windows sdk to use. Value defaults to "10.0.19041.0". Set to "." to use the Universal Kit
exit /B
