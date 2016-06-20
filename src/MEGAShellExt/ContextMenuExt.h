/****************************** Module Header ******************************\
Module Name:  FileContextMenuExt.h
Project:      CppShellExtContextMenuHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates creating a Shell context menu handler with C++. 

A context menu handler is a shell extension handler that adds commands to an 
existing context menu. Context menu handlers are associated with a particular 
file class and are called any time a context menu is displayed for a member 
of the class. While you can add items to a file class context menu with the 
registry, the items will be the same for all members of the class. By 
implementing and registering such a handler, you can dynamically add items to 
an object's context menu, customized for the particular object.

The example context menu handler adds the menu item "Display File Name (C++)"
to the context menu when you right-click a .cpp file in the Windows Explorer. 
Clicking the menu item brings up a message box that displays the full path 
of the .cpp file.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#ifndef CONTEXTMENUEXT_H
#define CONTEXTMENUEXT_H

#include <windows.h>
#include <shlobj.h>
#include <uxtheme.h>
#include <vector>
#include <string>

typedef HRESULT (WINAPI *pGetBufferedPaintBits) (HPAINTBUFFER hBufferedPaint, RGBQUAD **ppbBuffer, int *pcxRow);
typedef HPAINTBUFFER (WINAPI *pBeginBufferedPaint) (HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
typedef HRESULT (WINAPI *pEndBufferedPaint) (HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);

class ContextMenuExt : public IShellExtInit, public IContextMenu3
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);

    IFACEMETHODIMP HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    IFACEMETHODIMP HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

    ContextMenuExt(void);

protected:
    ~ContextMenuExt(void);
    bool isSynced(int type, int state);
    bool isUnsynced(int type, int state);
    void processFile(HDROP hDrop, int i);

private:
    // Reference count of component.
    long m_cRef;

    // The name of the selected file.
    //wchar_t m_szSelectedFile[MAX_PATH];
    std::vector<std::string> selectedFiles;
    std::vector<int> pathStates;
    std::vector<int> pathTypes;
    int syncedFolders, syncedFiles, syncedUnknowns;
    int unsyncedFolders, unsyncedFiles, unsyncedUnknowns;

    // The method that handles the "display" verb.
    void requestUpload();
    void requestGetLinks();
    HBITMAP getBitmap(HICON icon);
    HBITMAP getBitmapLegacy(HICON hIcon);
    bool legacyIcon;

    pGetBufferedPaintBits GetBufferedPaintBits;
    pBeginBufferedPaint BeginBufferedPaint;
    pEndBufferedPaint EndBufferedPaint;

    HBITMAP m_hMenuBmp;
    HICON hIcon;

    PCWSTR m_pszUploadMenuText;
    PCSTR m_pszUploadVerb;
    PCWSTR m_pwszUploadVerb;
    PCSTR m_pszUploadVerbCanonicalName;
    PCWSTR m_pwszUploadVerbCanonicalName;
    PCSTR m_pszUploadVerbHelpText;
    PCWSTR m_pwszUploadVerbHelpText;

    PCWSTR m_pszGetLinkMenuText;
    PCSTR m_pszGetLinkVerb;
    PCWSTR m_pwszGetLinkVerb;
    PCSTR m_pszGetLinkVerbCanonicalName;
    PCWSTR m_pwszGetLinkVerbCanonicalName;
    PCSTR m_pszGetLinkVerbHelpText;
    PCWSTR m_pwszGetLinkVerbHelpText;
};

#endif
