#include "Installer.h"

#include "AclHelper.h"
#include "PathHelper.h"
#include "RegistryKey.h"

#define GUID_STRING_SIZE 40

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Management::Deployment;

extern HMODULE g_hInst;

const wstring SparsePackageName = L"MEGASyncShellSparse";
constexpr int FirstWindows11BuildNumber = 22000;

bool IsWindows11Installation()
{
    RegistryKey registryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
    wstring buildNumberString = registryKey.GetStringValue(L"CurrentBuildNumber");

    const int buildNumber = stoi(buildNumberString);

    return buildNumber >= FirstWindows11BuildNumber;
}

HRESULT MoveFileToTempAndScheduleDeletion(const wstring& filePath, bool moveToTempDirectory)
{
    // Recommended way to check if a file exist:
    // https://devblogs.microsoft.com/oldnewthing/20071023-00/?p=24713
    DWORD fileAttributes = GetFileAttributesW(filePath.c_str());

    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        // If GetFileAttributes return INVALID_FILE_ATTRIBUTES, that means the file doesn't exist.
        // In that case, we shouldn't try and schedule a deletion of it.
        return S_OK;
    }

    wstring tempPath(MAX_PATH, L'\0');
    wstring tempFileName(MAX_PATH, L'\0');

    BOOL moveResult;

    if (moveToTempDirectory)
    {
        // First we get the path to the temporary directory.
        GetTempPath(MAX_PATH, &tempPath[0]);

        // Then we get a temporary filename in the temporary directory.
        GetTempFileName(tempPath.c_str(), L"tempFileName", 0, &tempFileName[0]);

        // Move the file into the temp directory - it can be moved even when it is loaded into
        // memory and locked.
        moveResult = MoveFileEx(filePath.c_str(), tempFileName.c_str(), MOVEFILE_REPLACE_EXISTING);

        if (!moveResult)
        {
            return S_FALSE;
        }

        // Schedule it to be deleted from the temp directory on the next reboot.
        moveResult = MoveFileExW(tempFileName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
    else
    {
        // Schedule it to be deleted on the next reboot, without moving it.
        moveResult = MoveFileExW(filePath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }

    if (!moveResult)
    {
        return S_FALSE;
    }

    return S_OK;
}

void ResetAclPermissionsOnApplicationFolder()
{
    // First we get the path where Notepad++ is installed.
    const wstring applicationPath = GetApplicationPath();

    // Create a new AclHelper
    AclHelper aclHelper;

    // Reset the ACL of the folder where Notepad++ is installed.
    aclHelper.ResetAcl(applicationPath);
}

Package GetSparsePackage()
{
    PackageManager packageManager;
    IIterable<Package> packages;

    try
    {
        packages = packageManager.FindPackagesForUser(L"");
    }
    catch (winrt::hresult_error)
    {
        return NULL;
    }

    for (const Package& package: packages)
    {
        if (package.Id().Name() != SparsePackageName)
        {
            continue;
        }

        return package;
    }

    return NULL;
}

HRESULT RegisterSparsePackage()
{
    if (::GetSystemMetrics(SM_CLEANBOOT) > 0)
    {
        return S_FALSE; // Otherwise we will get an unhandled exception later due to HRESULT
                        // 0x8007043c (ERROR_NOT_SAFEBOOT_SERVICE).
    }

    PackageManager packageManager;
    AddPackageOptions options;

    const wstring externalLocation = GetContextMenuPath();
    const wstring sparsePkgPath = externalLocation + L"\\ShellExtX64.msix";

    winrt::Windows::Foundation::Uri externalUri(externalLocation);
    winrt::Windows::Foundation::Uri packageUri(sparsePkgPath);

    options.ExternalLocationUri(externalUri);

    auto deploymentOperation = packageManager.AddPackageByUriAsync(packageUri, options);
    auto deployResult = deploymentOperation.get();

    if (!SUCCEEDED(deployResult.ExtendedErrorCode()))
    {
        return deployResult.ExtendedErrorCode();
    }

    return S_OK;
}

HRESULT UnregisterSparsePackage()
{
    if (::GetSystemMetrics(SM_CLEANBOOT) > 0)
    {
        return S_FALSE; // Only to speed up things a bit here. (code in the following
                        // GetSparsePackage() is safe against the ERROR_NOT_SAFEBOOT_SERVICE)
    }

    PackageManager packageManager;
    IIterable<Package> packages;

    Package package = GetSparsePackage();

    if (package == NULL)
    {
        return S_FALSE;
    }

    winrt::hstring fullName = package.Id().FullName();
    auto deploymentOperation = packageManager.RemovePackageAsync(fullName, RemovalOptions::None);
    auto deployResult = deploymentOperation.get();

    if (!SUCCEEDED(deployResult.ExtendedErrorCode()))
    {
        return deployResult.ExtendedErrorCode();
    }

    // After unregistering the sparse package, we reset the folder permissions of the folder where
    // we are installed.
    ResetAclPermissionsOnApplicationFolder();

    return S_OK;
}

void ReRegisterSparsePackage()
{
    if (::GetSystemMetrics(SM_CLEANBOOT) > 0)
    {
        return; // Sparse package reg/unreg cannot be done in the Windows OS SafeMode.
    }

    winrt::init_apartment();

    // Since we are on Windows 11, we unregister the sparse package as well.
    UnregisterSparsePackage();

    // And then we register it again.
    RegisterSparsePackage();
}

HRESULT InstallSparsePackage()
{
    const bool isWindows11 = IsWindows11Installation();

    HRESULT result;

    if (isWindows11)
    {
        // To register the sparse package, we need to do it on another thread due to WinRT
        // requirements.
        thread reRegisterThread(ReRegisterSparsePackage);
        reRegisterThread.join();
    }

    result = S_OK;

    // Finally we notify the shell that we have made changes, so it refreshes the context menus.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

    return result;
}

HRESULT UninstallSparsePackage()
{
    const bool isWindows11 = IsWindows11Installation();

    HRESULT result;

    // We remove the old context menu in all cases, since both Windows 11 and older versions can
    // have it setup if upgrading.
    result = S_OK;

    if (result != S_OK)
    {
        return result;
    }

    if (isWindows11)
    {
        // Since we are on Windows 11, we unregister the sparse package as well.
        result = UnregisterSparsePackage();

        if (result != S_OK)
        {
            return result;
        }
    }

    // Finally we notify the shell that we have made changes, so it refreshes the context menus.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

    return S_OK;
}

void EnsureRegistrationOnCurrentUserWorker()
{
    // Initialize the WinRT apartment.
    winrt::init_apartment();

    // Get the package to check if it is already installed for the current user.
    Package existingPackage = GetSparsePackage();

    if (existingPackage == NULL)
    {
        // The package is not installed for the current user - but we know that Notepad++ is.
        // If it wasn't, this code wouldn't be running, so it is safe to just register the package.
        RegisterSparsePackage();

        // Finally we notify the shell that we have made changes, so it reloads the right click menu
        // items.
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
    }
}

void EnsureRegistrationOnCurrentUser()
{
    // First we find the name of the process the DLL is being loaded into.
    wstring moduleName = GetExecutingModuleName();

    if (moduleName == L"explorer.exe")
    {
        const bool isWindows11 = IsWindows11Installation();

        if (isWindows11)
        {
            // We are being loaded into explorer.exe, so we can continue.
            // Explorer.exe only loads the DLL on the first time a user right-clicks a file
            // after that it stays in memory for the rest of their session.
            // Since we are here, we spawn a thread and call the
            // EnsureRegistrationOnCurrentUserWorker function.
            thread ensureRegistrationThread = thread(EnsureRegistrationOnCurrentUserWorker);
            ensureRegistrationThread.detach();
        }
    }
}

STDAPI CleanupDll()
{
    // First we get the full path to this DLL.
    wstring currentFilePath(MAX_PATH, L'\0');
    GetModuleFileName(g_hInst, &currentFilePath[0], MAX_PATH);

    // Then we get it moved out of the way and scheduled for deletion.
    return MoveFileToTempAndScheduleDeletion(currentFilePath, true);
}
