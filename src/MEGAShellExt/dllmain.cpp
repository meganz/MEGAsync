#include <windows.h>
#include <Guiddef.h>
#include "ClassFactoryContextMenuExt.h"
#include "ClassFactoryShellExtSynced.h" 
#include "ClassFactoryShellExtPending.h"
#include "ClassFactoryShellExtSyncing.h" 
#include "RegUtils.h"

#include <new>

STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);

// {0229E5E7-09E9-45CF-9228-0228EC7D5F17}
const CLSID CLSID_ContextMenuExt = 
{ 0x229e5e7, 0x9e9, 0x45cf, { 0x92, 0x28, 0x2, 0x28, 0xec, 0x7d, 0x5f, 0x17 } };
const TCHAR ContextMenuExtFriendlyName[] = L"###MegaContextMenuExt";

// {05B38830-F4E9-4329-978B-1DD28605D202}
const CLSID CLSID_ShellExtSynced = 
{ 0x5B38830, 0xF4E9, 0x4329, { 0x97, 0x8B, 0x1D, 0xD2, 0x86, 0x05, 0xD2, 0x02 } };
const TCHAR ShellExtSyncedFriendlyNameOld[] = L"###MegaShellExtSynced";
const TCHAR ShellExtSyncedFriendlyName[] = L"\x01 MEGA (Synced)";

//{056D528D-CE28-4194-9BA3-BA2E9197FF8C}
const CLSID CLSID_ShellExtPending = 
{ 0x56D528D, 0xCE28, 0x4194, { 0x9B, 0xA3, 0xBA, 0x2E, 0x91, 0x97, 0xFF, 0x8C } };
const TCHAR ShellExtPendingFriendlyNameOld[] = L"###MegaShellExtPending";
const TCHAR ShellExtPendingFriendlyName[] = L"\x01 MEGA (Pending)";

// {0596C850-7BDD-4C9D-AFDF-873BE6890637}
const CLSID CLSID_ShellExtSyncing =
{ 0x596c850, 0x7bdd, 0x4c9d, { 0xaf, 0xdf, 0x87, 0x3b, 0xe6, 0x89, 0x6, 0x37 } };
const TCHAR ShellExtSyncingFriendlyNameOld[] = L"###MegaShellExtSyncing";
const TCHAR ShellExtSyncingFriendlyName[] = L"\x01 MEGA (Syncing)";

HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
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
    if (IsEqualCLSID(CLSID_ContextMenuExt, rclsid))
    {
        hr = E_OUTOFMEMORY;
        pClassFactory = new (std::nothrow) ClassFactoryContextMenuExt();
    }
    else if (IsEqualCLSID(CLSID_ShellExtSynced, rclsid))
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
        hr = RegisterInprocServer(szModule, CLSID_ContextMenuExt,
            ContextMenuExtFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr)) return hr;

        // Register the context menu handler. The context menu handler is
        // associated with the .cpp file class.
        hr = RegisterShellExtContextMenuHandler(L"*",
            CLSID_ContextMenuExt,
            ContextMenuExtFriendlyName);
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtSynced,
            ShellExtSyncedFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterShellExtIconHandler(CLSID_ShellExtSynced,
            ShellExtSyncedFriendlyName);
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtPending,
            ShellExtPendingFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterShellExtIconHandler(CLSID_ShellExtPending,
            ShellExtPendingFriendlyName);
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterInprocServer(szModule, CLSID_ShellExtSyncing,
            ShellExtSyncingFriendlyName,
            L"Apartment");
        if (!SUCCEEDED(hr)) return hr;

        hr = RegisterShellExtIconHandler(CLSID_ShellExtSyncing,
            ShellExtSyncingFriendlyName);

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
        hr = UnregisterInprocServer(CLSID_ContextMenuExt);
        if (!SUCCEEDED(hr)) return hr;

        // Unregister the context menu handler.
        hr = UnregisterShellExtContextMenuHandler(L"*",
            CLSID_ContextMenuExt, ContextMenuExtFriendlyName);
        if (!SUCCEEDED(hr)) return hr;

        hr = UnregisterInprocServer(CLSID_ShellExtSynced);
        if (!SUCCEEDED(hr)) return hr;;

        hr = UnregisterInprocServer(CLSID_ShellExtPending);
        if (!SUCCEEDED(hr)) return hr;

        hr = UnregisterInprocServer(CLSID_ShellExtSyncing);
        if (!SUCCEEDED(hr)) return hr;

        UnregisterShellExtIconHandler(CLSID_ShellExtSynced, ShellExtSyncedFriendlyNameOld);
        UnregisterShellExtIconHandler(CLSID_ShellExtSynced, ShellExtSyncedFriendlyName);
        UnregisterShellExtIconHandler(CLSID_ShellExtPending, ShellExtPendingFriendlyNameOld);
        UnregisterShellExtIconHandler(CLSID_ShellExtPending, ShellExtPendingFriendlyName);
        UnregisterShellExtIconHandler(CLSID_ShellExtSyncing, ShellExtSyncingFriendlyNameOld);
        UnregisterShellExtIconHandler(CLSID_ShellExtSyncing, ShellExtSyncingFriendlyName);
        return hr;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_UNEXPECTED;
}
