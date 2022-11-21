"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_UNINSTALLER /DBUILD_X64_VERSION installer_win.nsi
UninstallerGenerator.exe
erase UninstallerGenerator.exe
copy uninst.exe built64
move uninst.exe sign64

"C:\Program Files (x86)\NSIS\makensis.exe" /DBUILD_UNINSTALLER installer_win.nsi
UninstallerGenerator.exe
erase UninstallerGenerator.exe
copy uninst.exe built32
move uninst.exe sign32


