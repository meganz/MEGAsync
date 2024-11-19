#include "Installer.h"

#include "AclSetter.h"
#include "PathHelper.h"
#include "RegistryKey.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Management::Deployment;

extern HMODULE g_hInst;

const std::wstring SparsePackageName = L"MEGASyncShellSparse";
constexpr int FirstWindows11BuildNumber = 22000;

bool IsWindows11Installation()
{
    RegistryKey registryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
    std::wstring buildNumberString = registryKey.GetStringValue(L"CurrentBuildNumber");

    const int buildNumber = stoi(buildNumberString);

    return buildNumber >= FirstWindows11BuildNumber;
}

void ResetAclPermissionsOnApplicationFolder()
{
    // First we get the path where the app is installed.
    const std::wstring applicationPath = GetApplicationPath();

    // Reset the ACL of the folder where the app is installed.
    AclSetter::resetAcl(applicationPath);
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

    const std::wstring externalLocation = GetContextMenuPath();
    const std::wstring sparsePkgPath = externalLocation + L"\\ShellExtX64.msix";

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
        std::thread reRegisterThread(ReRegisterSparsePackage);
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
    std::wstring moduleName = GetExecutingModuleName();

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
            std::thread ensureRegistrationThread =
                std::thread(EnsureRegistrationOnCurrentUserWorker);
            ensureRegistrationThread.detach();
        }
    }
}
