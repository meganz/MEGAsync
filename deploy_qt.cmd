SET MEGA_QT_DEPLOY_DIR=qt-deploy-x64

IF [%MEGA_QTPATH%]==[] (
	IF NOT [%MEGAQTPATH%]==[] (
		SET MEGA_QTPATH=%MEGAQTPATH%
	) ELSE (
		SET MEGA_QTPATH=C:\Qt\5.15.17\x64
	)
)

REM Clean up any previous leftovers
IF EXIST %MEGA_QT_DEPLOY_DIR% (
    rmdir /s /q %MEGA_QT_DEPLOY_DIR%
)

mkdir %MEGA_QT_DEPLOY_DIR%
%MEGA_QTPATH%\bin\windeployqt.exe --no-translations --no-compiler-runtime ^
 --no-system-d3d-compiler ^
 --no-webkit2 --no-qmltooling ^
 --no-patchqt --no-designercomponents ^
 --dir %MEGA_QT_DEPLOY_DIR% ^
 --qmldir src\MEGASync\gui\qml\ ^
 build-x64-windows-mega\src\MEGASync\RelWithDebInfo\MEGAsync.exe
rmdir /s /q %MEGA_QT_DEPLOY_DIR%\bearer
rmdir /s /q %MEGA_QT_DEPLOY_DIR%\scenegraph

REM Copy sofware opengl dll
copy %MEGA_QTPATH%\bin\opengl32sw.dll %MEGA_QT_DEPLOY_DIR%\opengl32sw.dll

IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
	GOTO :EOF
)

SET MEGA_QT_DEPLOY_DIR=qt-deploy-x86

REM Clean up any previous leftovers
IF EXIST %MEGA_QT_DEPLOY_DIR% (
    rmdir /s /q %MEGA_QT_DEPLOY_DIR%
)

mkdir %MEGA_QT_DEPLOY_DIR%
%MEGA_QTPATH%\..\x86\bin\windeployqt.exe --no-translations --no-compiler-runtime ^
 --no-system-d3d-compiler ^
 --no-webkit2 --no-qmltooling ^
 --no-patchqt --no-designercomponents ^
 --dir %MEGA_QT_DEPLOY_DIR% ^
 --qmldir src\MEGASync\gui\qml\ ^
 build-x86-windows-mega\src\MEGASync\RelWithDebInfo\MEGAsync.exe
rmdir /s /q %MEGA_QT_DEPLOY_DIR%\bearer
rmdir /s /q %MEGA_QT_DEPLOY_DIR%\scenegraph

REM Copy sofware opengl dll
copy %MEGA_QTPATH%\..\x86\bin\opengl32sw.dll %MEGA_QT_DEPLOY_DIR%\opengl32sw.dll