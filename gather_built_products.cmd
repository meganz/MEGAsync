
IF [%MEGA_VCPKGPATH%]==[] (
	SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty_MSVC2019_20230710\3rdParty_desktop
)

IF [%MEGA_THIRD_PARTY_DLL_DIR%]==[] (
	SET MEGA_THIRD_PARTY_DLL_DIR=bin_dlls_signed
)

mkdir built64
mkdir sign64

copy build-x64-windows-mega\Release\*.dll built64
copy build-x64-windows-mega\Release\*.exe built64
copy build-x64-windows-mega\Release\*.pdb built64

copy build-x64-windows-mega\Release\MEGAsync.exe sign64
copy build-x64-windows-mega\Release\MEGAupdater.exe sign64
copy build-x64-windows-mega\Release\MEGAShellExt.dll sign64

copy %MEGA_VCPKGPATH%\vcpkg\installed\x64-windows-mega\%MEGA_THIRD_PARTY_DLL_DIR%\*.* built64

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

mkdir built32
mkdir sign32

copy build-x86-windows-mega\Release\*.dll built32
copy build-x86-windows-mega\Release\*.exe built32
copy build-x86-windows-mega\Release\*.pdb built32

copy build-x86-windows-mega\Release\MEGAsync.exe sign32
copy build-x86-windows-mega\Release\MEGAupdater.exe sign32
copy build-x86-windows-mega\Release\MEGAShellExt.dll sign32

copy %MEGA_VCPKGPATH%\vcpkg\installed\x86-windows-mega\%MEGA_THIRD_PARTY_DLL_DIR%\*.* built32
