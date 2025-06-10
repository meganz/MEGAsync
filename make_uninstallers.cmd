SET SUFFIX_DEF=
if not "%MEGA_VERSION_SUFFIX%" == "" (
	SET SUFFIX_DEF=/DVERSION_SUFFIX=%MEGA_VERSION_SUFFIX%
)

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.17\x64
	)
)

"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_UNINSTALLER /DBUILD_X64_VERSION %SUFFIX_DEF% /DMEGA_QTPATH=%MEGA_QTPATH% /DWINKITVER=%MEGA_WIN_KITVER% installer_win.nsi || exit 1 /b
UninstallerGenerator.exe
erase UninstallerGenerator.exe
move uninst.exe built64 || exit 1 /b

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_UNINSTALLER %SUFFIX_DEF% /DMEGA_QTPATH=%MEGA_QTPATH% /DWINKITVER=%MEGA_WIN_KITVER% installer_win.nsi || exit 1 /b
UninstallerGenerator.exe
erase UninstallerGenerator.exe
move uninst.exe built32 || exit 1 /b
