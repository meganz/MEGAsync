IF [%MEGA_VCPKGPATH%]==[] (
	SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty_MSVC2019_20230710\3rdParty_desktop
)


REM Clean up any previous leftovers
IF EXIST build-x64-windows-mega (
    rmdir /s /q build-x64-windows-mega
)

mkdir build-x64-windows-mega
cd build-x64-windows-mega
cmake -G "Visual Studio 16 2019" -A x64 -DMega3rdPartyDir=%MEGA_VCPKGPATH% -DCMAKE_PREFIX_PATH=%MEGA_QTPATH% -DVCPKG_TRIPLET=x64-windows-mega -S "..\contrib\cmake" -B .
cmake --build . --config Release --target MEGAsync --target MEGAUpdater --target MEGAShellExt
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
cmake -G "Visual Studio 16 2019" -A Win32 -DMega3rdPartyDir=%MEGA_VCPKGPATH% -DCMAKE_PREFIX_PATH=%MEGA_QTPATH% -DVCPKG_TRIPLET=x86-windows-mega -S "..\contrib\cmake" -B .
cmake --build . --config Release --target MEGAsync --target MEGAUpdater --target MEGAShellExt
cd ..
