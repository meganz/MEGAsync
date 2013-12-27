/****************************** Module Header ******************************\
Module Name:  FileContextMenuExt.cpp
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

#include "ContextMenuExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


extern HINSTANCE g_hInst;
extern long g_cDllRef;

#define IDM_DISPLAY             0  // The command's identifier offset

ContextMenuExt::ContextMenuExt(void) : m_cRef(1), 
    m_pszMenuText(L"&Upload to MEGA"),
    m_pszVerb("UploadToMEGA"),
    m_pwszVerb(L"UploadToMEGA"),
    m_pszVerbCanonicalName("UploadToMEGA"),
    m_pwszVerbCanonicalName(L"UploadToMEGA"),
    m_pszVerbHelpText("Upload to MEGA"),
    m_pwszVerbHelpText(L"Upload to MEGA")
{
    InterlockedIncrement(&g_cDllRef);

    legacyIcon = true;
    HMODULE UxThemeDLL = LoadLibrary(L"UXTHEME.DLL");
    if (UxThemeDLL)
    {
        GetBufferedPaintBits = (pGetBufferedPaintBits)::GetProcAddress(UxThemeDLL, "GetBufferedPaintBits");
        BeginBufferedPaint = (pBeginBufferedPaint)::GetProcAddress(UxThemeDLL, "BeginBufferedPaint");
        EndBufferedPaint = (pEndBufferedPaint)::GetProcAddress(UxThemeDLL, "EndBufferedPaint");
        if(GetBufferedPaintBits && BeginBufferedPaint && EndBufferedPaint)
            legacyIcon = false;
        else
            FreeLibrary(UxThemeDLL);
    }

    hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON4), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_hMenuBmp = legacyIcon ? getBitmapLegacy(hIcon) : getBitmap(hIcon);
}

HBITMAP ContextMenuExt::getBitmapLegacy(HICON hIcon)
{
    RECT rect;
    rect.right = ::GetSystemMetrics(SM_CXMENUCHECK);
    rect.bottom = ::GetSystemMetrics(SM_CYMENUCHECK);
    rect.left = rect.top  = 0;

    // Create a compatible DC
    HDC dst_hdc = ::CreateCompatibleDC(NULL);
    if (dst_hdc == NULL) return NULL;

    // Create a new bitmap of icon size
    HBITMAP bmp = ::CreateCompatibleBitmap(dst_hdc, rect.right, rect.bottom);
    if (bmp == NULL)
    {
        DeleteDC(dst_hdc);
        return NULL;
    }

    SelectObject(dst_hdc, bmp);
    SetBkColor(dst_hdc, RGB(255, 255, 255));
    ExtTextOut(dst_hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
    DrawIconEx(dst_hdc, 0, 0, hIcon, rect.right, rect.bottom, 0, NULL, DI_NORMAL);
    DeleteDC(dst_hdc);
    return bmp;
}

HBITMAP ContextMenuExt::getBitmap(HICON icon)
{
    HBITMAP bitmap = NULL;
    HDC hdc = CreateCompatibleDC(NULL);
    if (!hdc) return NULL;

    int width   = GetSystemMetrics(SM_CXSMICON);
    int height  = GetSystemMetrics(SM_CYSMICON);
    RECT rect   = {0, 0, width, height};

    BITMAPINFO bmInfo = {0};
    bmInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biPlanes      = 1;
    bmInfo.bmiHeader.biCompression = BI_RGB;
    bmInfo.bmiHeader.biWidth       = width;
    bmInfo.bmiHeader.biHeight      = height;
    bmInfo.bmiHeader.biBitCount    = 32;

    bitmap = CreateDIBSection(hdc, &bmInfo, DIB_RGB_COLORS, NULL, NULL, 0);
    SelectObject(hdc, bitmap);
    BLENDFUNCTION blendFunction = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    BP_PAINTPARAMS paintParams = {0};
    paintParams.cbSize = sizeof(BP_PAINTPARAMS);
    paintParams.dwFlags = BPPF_ERASE;
    paintParams.pBlendFunction = &blendFunction;

    HDC newHdc;
    HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hdc, &rect, BPBF_DIB, &paintParams, &newHdc);
    if (hPaintBuffer)
    {
        DrawIconEx(newHdc, 0, 0, icon, width, height, 0, NULL, DI_NORMAL);
        EndBufferedPaint(hPaintBuffer, TRUE);
    }

    DeleteDC(hdc);
    return bitmap;
}

ContextMenuExt::~ContextMenuExt(void)
{
    if (m_hMenuBmp)
    {
        DeleteObject(m_hMenuBmp);
        m_hMenuBmp = NULL;
    }

    InterlockedDecrement(&g_cDllRef);
}


void ContextMenuExt::OnVerbDisplayFileName(HWND)
{
	for(unsigned int i=0; i<m_szSelectedFiles.size(); i++)
	{
		DWORD cbRead;
		LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\MEGApipe");
		CallNamedPipe(
			lpszPipename,														// pipe name
			(char *)m_szSelectedFiles[i].data(),								// message to server
			(lstrlen((LPCWSTR)m_szSelectedFiles[i].c_str())+1)*sizeof(TCHAR),			// message length
			NULL,																// buffer to receive reply
			0,																	// size of read buffer
			&cbRead,								// number of bytes read
			NMPWAIT_NOWAIT);						// waits
	}
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP ContextMenuExt::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(ContextMenuExt, IContextMenu),
        QITABENT(ContextMenuExt, IContextMenu2),
        QITABENT(ContextMenuExt, IContextMenu3),
        QITABENT(ContextMenuExt, IShellExtInit), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ContextMenuExt::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ContextMenuExt::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

#pragma endregion


#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP ContextMenuExt::Initialize(
	LPCITEMIDLIST , LPDATAOBJECT pDataObj, HKEY )
{
    if (NULL == pDataObj)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm;

    // The pDataObj pointer contains the objects being acted upon. In this 
    // example, we get an HDROP handle for enumerating the selected files and 
    // folders.
	m_szSelectedFiles.clear();
    if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
    {
        // Get an HDROP handle.
        HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
        if (hDrop != NULL)
        {
            UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			if (nFiles != 0)
            {
				LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\MEGApipe");
				DWORD cbRead = 0;
				TCHAR chReadBuf[2];
				TCHAR  lpszWrite[]=L"9";
				BOOL fSuccess = CallNamedPipe(
				   lpszPipename,							// pipe name
				   lpszWrite,								// message to server
				   sizeof(lpszWrite),						// message length
				   chReadBuf,								// buffer to receive reply
				   sizeof(chReadBuf),						// size of read buffer
				   &cbRead,									// number of bytes read
				   NMPWAIT_NOWAIT);

				if (fSuccess && cbRead!=0)
				{
					hr = S_OK;
					for(unsigned int i=0; i<nFiles; i++)
					{
						int characters = DragQueryFile(hDrop, i, NULL, 0);
						if(characters)
						{
							int prefixLenght = 2*sizeof(wchar_t);
							std::string buffer((const char *)L"F:", prefixLenght);
							buffer.resize((characters+1)*sizeof(wchar_t)+prefixLenght);
							int ok = DragQueryFile(hDrop, i, (LPWSTR)(buffer.data()+prefixLenght), (UINT)(characters+1));
							if(ok) m_szSelectedFiles.push_back(buffer);
						}
					}
				}
            }

            GlobalUnlock(stm.hGlobal);
        }

        ReleaseStgMedium(&stm);
    }

    // If any value other than S_OK is returned from the method, the context 
    // menu item is not displayed.
    return hr;
}

#pragma endregion


#pragma region IContextMenu

//
//   FUNCTION: FileContextMenuExt::QueryContextMenu
//
//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
//            context menu handler to add its menu items to the menu. It 
//            passes in the HMENU handle in the hmenu parameter. The 
//            indexMenu parameter is set to the index to be used for the 
//            first menu item that is to be added.
//
IFACEMETHODIMP ContextMenuExt::QueryContextMenu(
	HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT , UINT uFlags)
{
    // If uFlags include CMF_DEFAULTONLY then we should not do anything.
    if (CMF_DEFAULTONLY & uFlags)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
    }

    // Use either InsertMenu or InsertMenuItem to add menu items.
    // Learn how to add sub-menu from:
    // http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx

    MENUITEMINFO mii = { sizeof(mii) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mii.wID = idCmdFirst + IDM_DISPLAY;
    mii.fType = MFT_STRING;
    mii.dwTypeData = m_pszMenuText;
    mii.fState = MFS_ENABLED;
    mii.hbmpItem = legacyIcon ? HBMMENU_CALLBACK : m_hMenuBmp;

    if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Add a separator.
    MENUITEMINFO sep = { sizeof(sep) };
    sep.fMask = MIIM_TYPE;
    sep.fType = MFT_SEPARATOR;
    if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
    // Set the code value to the offset of the largest command identifier 
    // that was assigned, plus one (1).
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
}


//
//   FUNCTION: FileContextMenuExt::InvokeCommand
//
//   PURPOSE: This method is called when a user clicks a menu item to tell 
//            the handler to run the associated command. The lpcmi parameter 
//            points to a structure that contains the needed information.
//
IFACEMETHODIMP ContextMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    BOOL fUnicode = FALSE;

    // Determine which structure is being passed in, CMINVOKECOMMANDINFO or 
    // CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
    // the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
    // structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
    // structure. This struct is an extended version of CMINVOKECOMMANDINFO 
    // and has additional members that allow Unicode strings to be passed.
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if (pici->fMask & CMIC_MASK_UNICODE)
        {
            fUnicode = TRUE;
        }
    }

    // Determines whether the command is identified by its offset or verb.
    // There are two ways to identify commands:
    // 
    //   1) The command's verb string 
    //   2) The command's identifier offset
    // 
    // If the high-order word of lpcmi->lpVerb (for the ANSI case) or 
    // lpcmi->lpVerbW (for the Unicode case) is nonzero, lpVerb or lpVerbW 
    // holds a verb string. If the high-order word is zero, the command 
    // offset is in the low-order word of lpcmi->lpVerb.

    // For the ANSI case, if the high-order word is not zero, the command's 
    // verb string is in lpcmi->lpVerb. 
    if (!fUnicode && HIWORD(pici->lpVerb))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
        {
            OnVerbDisplayFileName(pici->hwnd);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    // For the Unicode case, if the high-order word is not zero, the 
    // command's verb string is in lpcmi->lpVerbW. 
    else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
        {
            OnVerbDisplayFileName(pici->hwnd);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    // If the command cannot be identified through the verb string, then 
    // check the identifier offset.
    else
    {
        // Is the command identifier offset supported by this context menu 
        // extension?
        if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
        {
            OnVerbDisplayFileName(pici->hwnd);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\MEGApipe");
	DWORD cbRead = 0;
	TCHAR chReadBuf[2];
	TCHAR  lpszWrite[]=L"9";
	CallNamedPipe(
	   lpszPipename,							// pipe name
	   lpszWrite,								// message to server
	   sizeof(lpszWrite),						// message length
	   chReadBuf,								// buffer to receive reply
	   sizeof(chReadBuf),						// size of read buffer
	   &cbRead,									// number of bytes read
	   NMPWAIT_NOWAIT);

    return S_OK;
}


//
//   FUNCTION: CFileContextMenuExt::GetCommandString
//
//   PURPOSE: If a user highlights one of the items added by a context menu 
//            handler, the handler's IContextMenu::GetCommandString method is 
//            called to request a Help text string that will be displayed on 
//            the Windows Explorer status bar. This method can also be called 
//            to request the verb string that is assigned to a command. 
//            Either ANSI or Unicode verb strings can be requested. This 
//            example only implements support for the Unicode values of 
//            uFlags, because only those have been used in Windows Explorer 
//            since Windows 2000.
//
IFACEMETHODIMP ContextMenuExt::GetCommandString(UINT_PTR idCommand, 
	UINT uFlags, UINT *, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;

    if (idCommand == IDM_DISPLAY)
    {
        switch (uFlags)
        {
        case GCS_HELPTEXTW:
            // Only useful for pre-Vista versions of Windows that have a 
            // Status bar.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbHelpText);
            break;

        case GCS_VERBW:
            // GCS_VERBW is an optional feature that enables a caller to 
            // discover the canonical name for the verb passed in through 
            // idCommand.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbCanonicalName);
            break;

        default:
            hr = S_OK;
        }
    }

    // If the command (idCommand) is not supported by this context menu 
    // extension handler, return E_INVALIDARG.

    return hr;
}

IFACEMETHODIMP ContextMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM, LPARAM lParam, LRESULT *plResult)
{
    LRESULT res;
    if (plResult == NULL)
        plResult = &res;
    *plResult = FALSE;

    switch (uMsg)
    {
    case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
            if (lpmis==NULL)
                break;
            lpmis->itemWidth = 16;
            lpmis->itemHeight = 16;
            *plResult = TRUE;
        }
        break;

    case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
            if ((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
                return S_OK;        //not for a menu

            DrawIconEx(lpdis->hDC,
                lpdis->rcItem.left-16,
                lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - 16) / 2,
                hIcon, 16, 16,
                0, NULL, DI_NORMAL);
            *plResult = TRUE;
        }
        break;
    }
    return S_OK;
}

IFACEMETHODIMP ContextMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

#pragma endregion
