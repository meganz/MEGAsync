mkdir built64

copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.pdb built64

copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.pdb built64
copy build-x64-windows-mega\src\MEGASync\RelWithDebInfo\*.rcc built64
copy build-x64-windows-mega\src\MEGASync\mega\RelWithDebInfo\*.pdb built64
copy build-x64-windows-mega\src\MEGASync\mega\tools\gfxworker\RelWithDebInfo\*.pdb built64

copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.dll built64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.exe built64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.pdb built64
copy build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.msix built64

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

mkdir built32

copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGAUpdater\RelWithDebInfo\*.pdb built32

copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.pdb built32
copy build-x86-windows-mega\src\MEGASync\RelWithDebInfo\*.rcc built32
copy build-x86-windows-mega\src\MEGASync\mega\RelWithDebInfo\*.pdb built32
copy build-x86-windows-mega\src\MEGASync\mega\tools\gfxworker\RelWithDebInfo\*.pdb built32

copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.dll built32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.exe built32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.pdb built32
copy build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo\*.msix built32
