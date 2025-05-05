#include "ClassFactoryContextMenuExt.h"
#include "ClassFactoryShellExtNotFound.h"
#include "ClassFactoryShellExtPending.h"
#include "ClassFactoryShellExtSynced.h"
#include "ClassFactoryShellExtSyncing.h"
#include "ContextMenuCommand.h"
#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandRemoveFromLeftPane.h"
#include "ContextMenuCommandSeparator.h"
#include "ContextMenuCommandUpload.h"
#include "ContextMenuCommandView.h"
#include "ContextMenuCommandViewVersions.h"
#include "RegUtils.h"
#include "SimpleFactory.h"
#include "SparsePackageManager.h"
#include "SubCommandEnumerator.h"
#include "Utilities.h"
#include <Guiddef.h>

#include <new>
#include <windows.h>

STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);

// {0229E5E7-09E9-45CF-9228-0228EC7D5F17}
const CLSID CLSID_ContextMenuExt = 
{ 0x229e5e7, 0x9e9, 0x45cf, { 0x92, 0x28, 0x2, 0x28, 0xec, 0x7d, 0x5f, 0x17 } };
const TCHAR ContextMenuExtFriendlyNameOld[] = L"###MegaContextMenuExt";
const TCHAR ContextMenuExtFriendlyName[] = L"MEGA (Context menu)";

// {05B38830-F4E9-4329-978B-1DD28605D202}
const CLSID CLSID_ShellExtSynced = 
{ 0x5B38830, 0xF4E9, 0x4329, { 0x97, 0x8B, 0x1D, 0xD2, 0x86, 0x05, 0xD2, 0x02 } };
const TCHAR ShellExtSyncedFriendlyNameOld[] = L"###MegaShellExtSynced";
const TCHAR ShellExtSyncedFriendlyName[] = L"\x01 MEGA (Synced)";

// {056D528D-CE28-4194-9BA3-BA2E9197FF8C}
const CLSID CLSID_ShellExtPending = 
{ 0x56D528D, 0xCE28, 0x4194, { 0x9B, 0xA3, 0xBA, 0x2E, 0x91, 0x97, 0xFF, 0x8C } };
const TCHAR ShellExtPendingFriendlyNameOld[] = L"###MegaShellExtPending";
const TCHAR ShellExtPendingFriendlyName[] = L"\x01 MEGA (Pending)";

// {0596C850-7BDD-4C9D-AFDF-873BE6890637}
const CLSID CLSID_ShellExtSyncing =
{ 0x596C850, 0x7BDD, 0x4C9D, { 0xAF, 0xDF, 0x87, 0x3B, 0xE6, 0x89, 0x6, 0x37 } };
const TCHAR ShellExtSyncingFriendlyNameOld[] = L"###MegaShellExtSyncing";
const TCHAR ShellExtSyncingFriendlyName[] = L"\x01 MEGA (Syncing)";

// {0596C850-7BDD-4C9D-AFDF-873BE6890635}
const CLSID CLSID_ShellExtNotFound =
{ 0x596C850, 0x7BDD, 0x4C9D, { 0xAF, 0xDF, 0x87, 0x3B, 0xE6, 0x89, 0x6, 0x35 } };
const TCHAR ShellExtNotFoundFriendlyNameOld[] = L"###MegaShellExtNotFound";
const TCHAR ShellExtNotFoundFriendlyName[] = L"\x01 MEGA (NotFound)";

HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;
winrt::com_ptr<SubCommandEnumerator> g_enumCommands = winrt::make_self<SubCommandEnumerator>();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        if (Utilities::haveModernContextMenu())
        {
            SparsePackageManager::EnsureRegistrationOnCurrentUser();
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    ClassFactory *pClassFactory = NULL;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    *ppv = NULL;
    if (Utilities::haveModernContextMenu())
    {
        if (rclsid == __uuidof(ContextMenuCommand))
        {
            hr = winrt::make<SimpleFactory<ContextMenuCommand>>().as(riid, ppv);
        }
    }
    else
    {
        if (IsEqualCLSID(CLSID_ContextMenuExt, rclsid))
        {
            hr = E_OUTOFMEMORY;
            pClassFactory = new (std::nothrow) ClassFactoryContextMenuExt();
        }
    }

    if (IsEqualCLSID(CLSID_ShellExtSynced, rclsid))
    {
        hr = E_OUTOFMEMORY;
        pClassFactory = new (std::nothrow) ClassFactoryShellExtSynced();
    }
    else if (IsEqualCLSID(CLSID_ShellExtPending, rclsid))
    {
        hr = E_OUTOFMEMORY;
        pClassFactory = new (std::nothrow) ClassFactoryShellExtPending();
    }
    else if (IsEqualCLSID(CLSID_ShellExtSyncing, rclsid))
    {
        hr = E_OUTOFMEMORY;
        pClassFactory = new (std::nothrow) ClassFactoryShellExtSyncing();
    }
    else if (IsEqualCLSID(CLSID_ShellExtNotFound, rclsid))
    {
        hr = E_OUTOFMEMORY;
        pClassFactory = new (std::nothrow) ClassFactoryShellExtNotFound();
    }

    if (pClassFactory)
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();
    }

    return hr;
}

//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference 
//   count is zero (i.e. nobody is still using the component).
// 
STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the context menu handler.
// 
STDAPI DllRegisterServer(void)
{
    __try
    {
        DllUnregisterServer();

        HRESULT hr = S_OK;

        TCHAR szModule[MAX_PATH];
        if (GetModuleFileName(g_hInst, szModule, MAX_PATH) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }

        // Register the component.
        if (Utilities::haveModernContextMenu())
        {
            SparsePackageManager::modifySparsePackage(SparsePackageManager::MODIFY_TYPE::INSTALL);
        }
        else
        {
            hr = RegisterInprocServer(szModule,
                                      CLSID_ContextMenuExt,
                                      ContextMenuExtFriendlyName,
                                      L"Apartment");
            if (!SUCCEEDED(hr))
                return hr;

            hr = RegisterShellExtContextMenuHandler(L"*",
                                                    CLSID_ContextMenuExt,
                                                    ContextMenuExtFriendlyName);
            if (!SUCCEEDED(hr))
                return hr;

            hr = RegisterShellExtContextMenuHandler(L"AllFilesystemObjects",
                                                    CLSID_ContextMenuExt,
                                                    ContextMenuExtFriendlyName);
            if (!SUCCEEDED(hr))
                return hr;

            hr = RegisterShellExtContextMenuHandler(L"Drive",
                                                    CLSID_ContextMenuExt,
                                                    ContextMenuExtFriendlyName);
            if (!SUCCEEDED(hr))
                return hr;
        }

        hr = RegisterInprocServer(szModule,
                                  CLSID_ShellExtSynced,
                                  ShellExtSyncedFriendlyName,
                                  L"Apartment");
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterShellExtIconOverlayHandler(CLSID_ShellExtSynced,
            ShellExtSyncedFriendlyName);
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtPending,
            ShellExtPendingFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterShellExtIconOverlayHandler(CLSID_ShellExtPending,
            ShellExtPendingFriendlyName);
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtSyncing,
            ShellExtSyncingFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterShellExtIconOverlayHandler(CLSID_ShellExtSyncing,
            ShellExtSyncingFriendlyName);
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtNotFound,
            ShellExtNotFoundFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr))
            return hr;

        hr = RegisterShellExtIconOverlayHandler(CLSID_ShellExtNotFound,
            ShellExtNotFoundFriendlyName);
        if (!SUCCEEDED(hr))
            return hr;

        return hr;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return E_UNEXPECTED;
}


//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the context menu handler.
// 
STDAPI DllUnregisterServer(void)
{
    __try
    {
        HRESULT hr = S_OK;

        TCHAR szModule[MAX_PATH];
        if (GetModuleFileName(g_hInst, szModule, MAX_PATH) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }

        // Unregister the component.
        if (Utilities::haveModernContextMenu())
        {
            SparsePackageManager::modifySparsePackage(SparsePackageManager::MODIFY_TYPE::UNINSTALL);
        }
        else
        {
            hr = UnregisterInprocServer(CLSID_ContextMenuExt);
            if (!SUCCEEDED(hr))
                return hr;
        }

        hr = UnregisterInprocServer(CLSID_ShellExtSynced);
        if (!SUCCEEDED(hr)) return hr;;

        hr = UnregisterInprocServer(CLSID_ShellExtPending);
        if (!SUCCEEDED(hr)) return hr;

        hr = UnregisterInprocServer(CLSID_ShellExtSyncing);
        if (!SUCCEEDED(hr)) return hr;

        hr = UnregisterInprocServer(CLSID_ShellExtNotFound);
        if (!SUCCEEDED(hr)) return hr;

        if (!Utilities::haveModernContextMenu())
        {
            UnregisterShellExtContextMenuHandler(L"*",
                                                 CLSID_ContextMenuExt,
                                                 ContextMenuExtFriendlyNameOld);
            UnregisterShellExtContextMenuHandler(L"*",
                                                 CLSID_ContextMenuExt,
                                                 ContextMenuExtFriendlyName);
            UnregisterShellExtContextMenuHandler(L"AllFilesystemObjects",
                                                 CLSID_ContextMenuExt,
                                                 ContextMenuExtFriendlyName);
            UnregisterShellExtContextMenuHandler(L"Drive",
                                                 CLSID_ContextMenuExt,
                                                 ContextMenuExtFriendlyName);
        }
        UnregisterShellExtOverlayHandler(CLSID_ShellExtSynced, ShellExtSyncedFriendlyNameOld);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtSynced, ShellExtSyncedFriendlyName);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtPending, ShellExtPendingFriendlyNameOld);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtPending, ShellExtPendingFriendlyName);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtSyncing, ShellExtSyncingFriendlyNameOld);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtSyncing, ShellExtSyncingFriendlyName);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtNotFound, ShellExtNotFoundFriendlyNameOld);
        UnregisterShellExtOverlayHandler(CLSID_ShellExtNotFound, ShellExtNotFoundFriendlyName);

        return hr;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_UNEXPECTED;
}
