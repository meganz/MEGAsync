
IF [%MEGA_VCPKGPATH%]==[] (
	SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty_MSVC2019_20230710\3rdParty_desktop
)

IF [%MEGA_THIRD_PARTY_DLL_DIR%]==[] (
	SET MEGA_THIRD_PARTY_DLL_DIR=bin_dlls_signed
)

mkdir built64
mkdir sign64

copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.pdb built64

copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.pdb built64
copy build-x64-windows-mega\src\MEGASync\mega\RelWithDebInfo\*.pdb built64

copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.pdb built64

copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\MEGAsync.exe sign64
copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\MEGAupdater.exe sign64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\MEGAShellExt.dll sign64

:: TODO: also copy unsigned dlls

:: copy %MEGA_VCPKGPATH%\vcpkg\installed\x64-windows-mega\%MEGA_THIRD_PARTY_DLL_DIR%\*.* built64

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

mkdir built32
mkdir sign32

copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.pdb built32

copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.pdb built32
copy build-x86-windows-mega\src\MEGASync\mega\RelWithDebInfo\*.pdb built32

copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.pdb built32

copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\MEGAsync.exe sign32
copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\MEGAupdater.exe sign32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\MEGAShellExt.dll sign32

:: TODO: also copy unsigned dlls

:: copy %MEGA_VCPKGPATH%\vcpkg\installed\x86-windows-mega\%MEGA_THIRD_PARTY_DLL_DIR%\*.* built32
