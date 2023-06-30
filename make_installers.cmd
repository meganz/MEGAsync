SET SUFFIX_DEF=
if not "%MEGA_VERSION_SUFFIX%" == "" (
	SET SUFFIX_DEF=/DVERSION_SUFFIX=%MEGA_VERSION_SUFFIX%
)

erase MEGAsyncSetup64.exe
"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_X64_VERSION %SUFFIX_DEF% installer_win.nsi

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

erase MEGAsyncSetup32.exe
"C:\Program Files (x86)\NSIS\makensis.exe" %SUFFIX_DEF% installer_win.nsi
