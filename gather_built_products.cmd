IF "%MEGA_CLEAN_BUILT_FOLDERS%" == "true" (
  REM Clean up any previous leftovers
  IF EXIST built64 (
    rmdir /s /q built64
  )
  IF EXIST sign64 (
    rmdir /s /q sign64
  )
)

IF [%MEGA_VCPKGPATH%]==[] (
    SET MEGA_VCPKGPATH=C:\Users\build\MEGA\build-MEGASync\3rdParty_MSVC2019_20221029\3rdParty_desktop
    rem SET MEGA_VCPKGPATH=%CD%\..\3rdparty_desktop
)

IF [%MEGA_THIRD_PARTY_DLL_DIR%]==[] (
    SET MEGA_THIRD_PARTY_DLL_DIR=bin_dlls_signed
    rem SET MEGA_THIRD_PARTY_DLL_DIR=bin
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

IF "%MEGA_CLEAN_BUILT_FOLDERS%" == "true" (
  IF EXIST built32 (
    rmdir /s /q built32
  )
  IF EXIST sign32 (
    rmdir /s /q sign32
  )
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
