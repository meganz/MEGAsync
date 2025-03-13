@echo off

:: Check if the first argument is "--help"
if "%1"=="--help" (
    echo Usage: sign_context_menu_products.cmd [option] [arguments]
    echo The default certificate subject name is "Mega Limited"
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

echo The files path is %DIR_PATH%

:: Define the store (can be "My" for Current User, "CA" for Certificate Authorities, etc.)
set CERT_STORE=My

:: Check if the certificate is installed in the specified store
echo Checking if the certificate with subject containing "Mega Limited" is installed in the "%CERT_STORE%" store...

::certutil -store "%CERT_STORE%" | findstr /i "*%CERT_SUBJECT%*" >nul
for /f "delims=" %%i in ('powershell -Command "$cert = Get-ChildItem -Path Cert:\CurrentUser\My| Where-Object { $_.Subject -match 'Mega Limited' }; if ($cert) { $cert.Thumbprint } else { 'Certificate not found' }"') do set Thumbprint=%%i

if errorlevel 1 (
    echo Certificate with subject containing "Mega Limited" not found in the "%CERT_STORE%" store.
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

:: Sign the DLL file
signtool sign /sha1 "%Thumbprint%" /td SHA256 /fd SHA256 %DLL_PATH%

:: Sign the MSIX file
signtool sign /sha1 "%Thumbprint%" /td SHA256 /fd SHA256 %MSIX_PATH%

:: Verify the signed files
signtool verify /pa "%DLL_PATH%"
signtool verify /pa "%MSIX_PATH%"

echo Files signed successfully.