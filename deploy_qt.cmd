SET MEGA_QT_DEPLOY_DIR=qt-deploy-x64

REM Clean up any previous leftovers
IF EXIST %MEGA_QT_DEPLOY_DIR% (
    rmdir /s /q %MEGA_QT_DEPLOY_DIR%
)

mkdir %MEGA_QT_DEPLOY_DIR%
C:\Qt\5.12.12\msvc2017_64\bin\windeployqt.exe --no-translations --no-compiler-runtime ^
 --no-opengl-sw --no-system-d3d-compiler ^
 --dir %MEGA_QT_DEPLOY_DIR% ^
 --qmldir src\MEGASync\gui\onboarding\qml\ ^
 build-x64-windows-mega\Release\MEGAsync.exe ^
 build-x64-windows-mega\Release\MEGAupdater.exe ^
 build-x64-windows-mega\Release\MEGAShellExt.dll

rmdir /s /q %MEGA_QT_DEPLOY_DIR%\bearer

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

SET MEGA_QT_DEPLOY_DIR=qt-deploy-x86

REM Clean up any previous leftovers
IF EXIST %MEGA_QT_DEPLOY_DIR% (
    rmdir /s /q %MEGA_QT_DEPLOY_DIR%
)

mkdir %MEGA_QT_DEPLOY_DIR%
C:\Qt\5.12.12\msvc2017\bin\windeployqt.exe --no-translations --no-compiler-runtime ^
 --no-opengl-sw --no-system-d3d-compiler ^
 --dir %MEGA_QT_DEPLOY_DIR% ^
 --qmldir src\MEGASync\gui\onboarding\qml ^
 build-x86-windows-mega\Release\MEGAsync.exe ^
 build-x86-windows-mega\Release\MEGAShellExt.dll
 build-x86-windows-mega\Release\MEGAupdater.exe ^

rmdir /s /q %MEGA_QT_DEPLOY_DIR%\bearer