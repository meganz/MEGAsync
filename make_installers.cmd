SET SUFFIX_DEF=
if not "%MEGA_VERSION_SUFFIX%" == "" (
	SET SUFFIX_DEF=/DVERSION_SUFFIX=%MEGA_VERSION_SUFFIX%
)

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.16\x64
	)
)

erase MEGAsyncSetup64.exe
"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_X64_VERSION %SUFFIX_DEF% /DMEGA_QTPATH=%MEGA_QTPATH% /DWINKITVER=%MEGA_WIN_KITVER% installer_win.nsi || exit 1 /b

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

erase MEGAsyncSetup32.exe
"C:\Program Files (x86)\NSIS\makensis.exe" %SUFFIX_DEF% /DMEGA_QTPATH=%MEGA_QTPATH% /DWINKITVER=%MEGA_WIN_KITVER% installer_win.nsi || exit 1 /b
