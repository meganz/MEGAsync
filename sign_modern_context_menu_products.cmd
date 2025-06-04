@echo off

:: To sign the .msix file, it is mandatory that the publisher of the AppxManifest.xml file
:: is identical to the subject of the certificate to be used for signing.
:: To achieve this, the AppxManifest.xml is created with a publisher taken from an environment variable called MEGA_DESKTOP_APP_CERTIFICATE_PUBLISHER.
:: Be careful because that publisher must comply with the xml rules, so you must escape the characters not allowed in xml.
:: For example, quotation marks (") will be replaced by quot;

:: Check if the first argument is "--help"
if "%1"=="--help" (
    echo Usage: sign_modern_context_menu_products.cmd [option] [arguments]
    echo The default certificate subject name is %CERT_SUBJECT%
    echo.
    echo Options:
    echo   --help       Display this help message
    echo   --path       Dll and msix path
    echo.
    exit /b
)

:: Check if the directory path argument was provided
if "%1"=="--path" (
    :: Set the directory path from the input argument
    set DIR_PATH=%2
)

:: Check if the directory path argument is empty
if "%DIR_PATH%"=="" (
    echo Error: path to the files cannot be empty
    exit /b 1
)

:: Check if the directory path argument is empty
if "%MEGA_DESKTOP_APP_CERTIFICATE_THUMBPRINT%"=="" (
    echo Error: Certificate thumbprint cannot be empty
    echo Create a MEGA_DESKTOP_APP_CERTIFICATE_THUMBPRINT environment variable
    exit /b 1
)

echo Working directory: %DIR_PATH%

:: Set the full paths to the .dll and .msix files (both are in the same directory)
set DLL_PATH=%DIR_PATH%\MEGAShellExt.dll
set MSIX_PATH=%DIR_PATH%\MEGAShellExt.msix

:: Check if the files exist
if not exist "%DLL_PATH%" (
    echo MEGAShellExt.dll file not found in the specified directory.
    exit /b
)

if not exist "%MSIX_PATH%" (
    echo MEGAShellExt.msix file not found in the specified directory.
    exit /b
)

echo.
:: Sign the DLL file
signtool sign /sha1 "%MEGA_DESKTOP_APP_CERTIFICATE_THUMBPRINT%" /td SHA256 /fd SHA256 %DLL_PATH%

:: Verify signature
signtool verify /pa "%DLL_PATH%"
if %errorlevel% neq 0 (
    echo Error: DLL signing has failed.
    exit /b %errorlevel%
)

echo.
:: Sign the MSIX file
"C:\Program Files (x86)\Windows Kits\10\bin\%MEGA_WIN_KITVER%\x64\signtool.exe" sign /sha1 "%MEGA_DESKTOP_APP_CERTIFICATE_THUMBPRINT%" /td SHA256 /fd SHA256 %MSIX_PATH%

:: Verify signature
"C:\Program Files (x86)\Windows Kits\10\bin\%MEGA_WIN_KITVER%\x64\signtool.exe" verify /pa "%MSIX_PATH%"
if %errorlevel% neq 0 (
    echo Error: DMSIX signing has failed.
    exit /b %errorlevel%
)

echo.
echo Files signed successfully.