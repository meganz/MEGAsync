mkdir built32
mkdir built64

copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaShellExt\release\*.dll built64
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaShellExt\release\*.pdb built64
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaUpdater\release\*.exe built64
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaUpdater\release\*.pdb built64
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaSync\release\*.exe built64
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_64bit-Release\MegaSync\release\*.pdb built64

copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaShellExt\release\*.dll built32
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaShellExt\release\*.pdb built32
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaUpdater\release\*.exe built32
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaUpdater\release\*.pdb built32
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaSync\release\*.exe built32
copy build-MEGA-Desktop_Qt_5_12_8_MSVC2017_32bit-Release\MegaSync\release\*.pdb built32

copy C:\Users\build\MEGA\build-MEGAsync\3rdParty_MSVC2017_20200529\vcpkg\installed\x64-windows-mega\bin_dlls_signed\*.* built64

copy C:\Users\build\MEGA\build-MEGAsync\3rdParty_MSVC2017_20200529\vcpkg\installed\x86-windows-mega\bin\*.* built32

