IF [%MEGA_VCPKGPATH%]==[] (
	SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty_MSVC2019_20230710\3rdParty_desktop
)

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.13\x64
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
cmake -DVCPKG_ROOT=%MEGA_VCPKGPATH%\vcpkg -DCMAKE_PREFIX_PATH=%MEGA_QTPATH% -S ".." -B . || exit 1 /b
cmake --build . --config RelWithDebInfo --target MEGAsync --target MEGAupdater --target MEGAShellExt -j%MEGA_CORES% || exit 1 /b
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
cmake -DCMAKE_GENERATOR_PLATFORM:UNINITIALIZED=Win32 -DVCPKG_ROOT=%MEGA_VCPKGPATH%\vcpkg -DCMAKE_PREFIX_PATH=%MEGA_QTPATH%\..\x86 -S ".." -B . || exit 1 /b
cmake --build . --config RelWithDebInfo --target MEGAsync --target MEGAupdater --target MEGAShellExt -j%MEGA_CORES% || exit 1 /b
cd ..
