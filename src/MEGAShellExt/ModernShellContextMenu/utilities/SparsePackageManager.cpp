#include "SparsePackageManager.h"

#include "Utilities.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation::Collections;

extern HMODULE g_hInst;

const std::wstring SparsePackageName = L"MEGASyncShellSparse";
#if _WIN32 || _WIN64
#if _WIN64
const std::wstring SparseInstallerName = L"\\ShellExtX64.msix";
#else
const std::wstring SparseInstallerName = L"\\ShellExtX32.msix";
#endif
#endif

winrt::Windows::ApplicationModel::Package SparsePackageManager::mSparsePackage = NULL;

void SparsePackageManager::findSparsePackage()
{
    if (mSparsePackage != NULL)
    {
        return;
    }

    PackageManager packageManager;
    IIterable<Package> packages;

    try
    {
        packages = packageManager.FindPackagesForUser(L"");
    }
    catch (winrt::hresult_error)
    {
        return;
    }

    for (const Package& package: packages)
    {
        if (package.Id().Name() == SparsePackageName)
        {
            mSparsePackage = package;
            break;
        }
    }
}

HRESULT SparsePackageManager::UnregisterSparsePackage()
{
    if (::GetSystemMetrics(SM_CLEANBOOT) > 0)
    {
        return S_FALSE;
    }

    findSparsePackage();

    if (mSparsePackage == NULL)
    {
        return S_FALSE;
    }

    PackageManager packageManager;
    auto deployResult =
        packageManager.RemovePackageAsync(mSparsePackage.Id().FullName(), RemovalOptions::None)
            .get();

    if (!SUCCEEDED(deployResult.ExtendedErrorCode()))
    {
        return deployResult.ExtendedErrorCode();
    }

    // After unregistering the sparse package, we reset the folder permissions of the folder where
    // we are installed.
    Utilities::resetAcl();

    return S_OK;
}

HRESULT SparsePackageManager::RegisterSparsePackage()
{
    if (::GetSystemMetrics(SM_CLEANBOOT) > 0)
    {
        return S_FALSE;
    }

    const std::wstring externalLocation = Utilities::GetContextMenuPath();
    const std::wstring sparsePkgPath = externalLocation + SparseInstallerName;

    winrt::Windows::Foundation::Uri externalUri(externalLocation);
    winrt::Windows::Foundation::Uri packageUri(sparsePkgPath);

    AddPackageOptions options;
    options.ExternalLocationUri(externalUri);

    auto errorCode{addPackage(packageUri, options)};

    if (!SUCCEEDED(errorCode.get()))
    {
        return errorCode.get();
    }

    return S_OK;
}

concurrency::task<HRESULT>
    SparsePackageManager::addPackage(const winrt::Windows::Foundation::Uri& packageUri,
                                     const AddPackageOptions& options)
{
    return concurrency::create_task(
        [packageUri, options]
        {
            PackageManager packageManager;
            auto deploy = packageManager.AddPackageByUriAsync(packageUri, options);
            auto deployResult = deploy.get();
            return HRESULT(deployResult.ExtendedErrorCode());
        });
}

void SparsePackageManager::ReRegisterSparsePackage()
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

HRESULT SparsePackageManager::modifySparsePackage(MODIFY_TYPE type)
{
    HRESULT result(S_OK);

    if (Utilities::haveModernContextMenu())
    {
        if (type == MODIFY_TYPE::INSTALL)
        {
            // To register the sparse package, we need to do it on another thread due to WinRT
            // requirements.
            std::thread reRegisterThread(&SparsePackageManager::ReRegisterSparsePackage);
            reRegisterThread.detach();
        }
        else
        {
            winrt::uninit_apartment();
            result = UnregisterSparsePackage();
        }
    }

    // Finally we notify the shell that we have made changes, so it refreshes the context menus.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

    return result;
}

void SparsePackageManager::EnsureRegistrationOnCurrentUserWorker()
{
    // Initialize the WinRT apartment.
    winrt::init_apartment();

    findSparsePackage();

    if (mSparsePackage == NULL)
    {
        // The package is not installed for the current user - but we know that Notepad++ is.
        // If it wasn't, this code wouldn't be running, so it is safe to just register the package.
        RegisterSparsePackage();

        // Finally we notify the shell that we have made changes, so it reloads the right click menu
        // items.
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
    }
}

void SparsePackageManager::EnsureRegistrationOnCurrentUser()
{
    // First we find the name of the process the DLL is being loaded into.
    std::wstring moduleName = Utilities::GetExecutingModuleName();

    if (moduleName == L"explorer.exe")
    {
        if (Utilities::haveModernContextMenu())
        {
            // We are being loaded into explorer.exe, so we can continue.
            // Explorer.exe only loads the DLL on the first time a user right-clicks a file
            // after that it stays in memory for the rest of their session.
            // Since we are here, we spawn a thread and call the
            // EnsureRegistrationOnCurrentUserWorker function.
            std::thread ensureRegistrationThread =
                std::thread(&SparsePackageManager::EnsureRegistrationOnCurrentUserWorker);
            ensureRegistrationThread.detach();
        }
    }
}
