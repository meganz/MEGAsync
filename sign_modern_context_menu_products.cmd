@echo off

:: To sign the .msix file, it is mandatory that the publisher of the AppxManifest.xml file
:: is identical to the subject of the certificate to be used for signing.
:: To achieve this, the AppxManifest.xml is created with a publisher taken from an environment variable called MEGA_CERTIFICATE_PUBLISHER.
:: Be careful because that publisher must comply with the xml rules, so you must escape the characters not allowed in xml.
:: For example, quotation marks (") will be replaced by quot;

:: Working on My certificate store
set CERT_STORE=My

:: Certificate subject (Change it in case you have a different certificate subject)
set CERT_SUBJECT="Mega Limited"

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

echo Working directory: %DIR_PATH%

:: Check if the certificate is installed in the specified store
echo Checking if the certificate with subject containing %CERT_SUBJECT% is installed in the "%CERT_STORE%" store...

for /f "delims=" %%i in ('powershell -Command "$cert = Get-ChildItem -Path Cert:\CurrentUser\My| Where-Object { $_.Subject -match '%CERT_SUBJECT%' }; if ($cert) { $cert.Thumbprint } else { 'Certificate not found' }"') do set Thumbprint=%%i

if errorlevel 1 (
    echo Certificate with subject containing %CERT_SUBJECT% not found in the "%CERT_STORE%" store.
    exit /b
) else (
    echo Certificate found. Proceeding with signing.
)

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
signtool sign /sha1 "%Thumbprint%" /td SHA256 /fd SHA256 %DLL_PATH%

:: Verify signature
signtool verify /pa "%DLL_PATH%"
if %errorlevel% neq 0 (
    echo Error: DLL signing has failed.
    exit /b %errorlevel%
)

echo.
:: Sign the MSIX file
signtool sign /sha1 "%Thumbprint%" /td SHA256 /fd SHA256 %MSIX_PATH%

:: Verify signature
signtool verify /pa "%MSIX_PATH%"
if %errorlevel% neq 0 (
    echo Error: DMSIX signing has failed.
    exit /b %errorlevel%
)

echo.
echo Files signed successfully.