mkdir built32
mkdir built64

copy build-x64-windows-mega\Release\*.dll built64
copy build-x64-windows-mega\Release\*.exe built64
copy build-x64-windows-mega\Release\*.pdb built64
copy build-x86-windows-mega\Release\*.dll built32
copy build-x86-windows-mega\Release\*.exe built32
copy build-x86-windows-mega\Release\*.pdb built32

copy C:\Users\build\MEGA\build-MEGAsync\3rdParty_MSVC2019_20211109\3rdparty_desktop\vcpkg\installed\x64-windows-mega\bin_dlls_signed\*.* built64
copy C:\Users\build\MEGA\build-MEGAsync\3rdParty_MSVC2019_20211109\3rdparty_desktop\vcpkg\installed\x86-windows-mega\bin_dlls_signed\*.* built32

