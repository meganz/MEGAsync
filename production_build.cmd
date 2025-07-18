IF [%MEGA_VCPKGPATH%]==[] (
	SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty\vcpkg
)

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.16\x64
	)
)

IF [%MEGA_CORES%]==[] (
	FOR /f "tokens=2 delims==" %%f IN ('wmic cpu get NumberOfLogicalProcessors /value ^| find "="') DO SET MEGA_CORES=%%f
)

REM Clean up any previous leftovers
IF EXIST build-x64-windows-mega (
    rmdir /s /q build-x64-windows-mega
)

mkdir build-x64-windows-mega
cd build-x64-windows-mega
REM Overwrite C and CXX flags. We want debug info, with O2 and Ob2 level optimization.
cmake ^
	-G "Visual Studio 17 2022" ^
	-A x64 ^
	-DCMAKE_SYSTEM_VERSION=%MEGA_WIN_KITVER% ^
	-DCMAKE_VERBOSE_MAKEFILE="ON" ^
	-DENABLE_DESIGN_TOKENS_IMPORTER="OFF" ^
	-DENABLE_DESKTOP_APP_TESTS="OFF" ^
	-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/Zi /O2 /Ob2 /DNDEBUG" ^
	-DCMAKE_C_FLAGS_RELWITHDEBINFO="/Zi /O2 /Ob2 /DNDEBUG" ^
	-DVCPKG_ROOT=%MEGA_VCPKGPATH% ^
	-DCMAKE_PREFIX_PATH=%MEGA_QTPATH% ^
	-S ".." ^
	-B . ^
	|| exit 1 /b
cmake ^
	--build . ^
	--config RelWithDebInfo ^
	--target MEGAsync ^
	--target MEGAupdater ^
	--target MEGAShellExt ^
	-j%MEGA_CORES% ^
	|| exit 1 /b
cd ..

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

REM Clean up any previous leftovers
IF EXIST build-x86-windows-mega (
	rmdir /s /q build-x86-windows-mega
)

mkdir build-x86-windows-mega
cd build-x86-windows-mega
REM Overwrite C and CXX flags. We want debug info, with O2 and Ob2 level optimization.
cmake ^
	-G "Visual Studio 17 2022" ^
	-A Win32 ^
	-DCMAKE_SYSTEM_VERSION=%MEGA_WIN_KITVER% ^
	-DCMAKE_VERBOSE_MAKEFILE="ON" ^
	-DENABLE_DESIGN_TOKENS_IMPORTER="OFF" ^
	-DENABLE_DESKTOP_APP_TESTS="OFF" ^
	-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/Zi /O2 /Ob2 /DNDEBUG" ^
	-DCMAKE_C_FLAGS_RELWITHDEBINFO="/Zi /O2 /Ob2 /DNDEBUG" ^
	-DVCPKG_ROOT=%MEGA_VCPKGPATH% ^
	-DCMAKE_PREFIX_PATH=%MEGA_QTPATH%\..\x86 ^
	-S ".." ^
	-B . ^
	|| exit 1 /b
cmake ^
	--build . ^
	--config RelWithDebInfo ^
	--target MEGAsync ^
	--target MEGAupdater ^
	--target MEGAShellExt ^
	-j%MEGA_CORES% ^
	|| exit 1 /b
cd ..
