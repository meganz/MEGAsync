#include "WindowsPlatform.h"
#include <Shlobj.h>
#include <Shlwapi.h>

#if QT_VERSION >= 0x050200
#include <QtWin>
#endif

// Windows headers don't define this for WinXP despite the documentation says that they should
// and it indeed works
#ifndef SHFOLDERCUSTOMSETTINGS
#include <pshpack8.h>

// Used by SHGetSetFolderCustomSettings
typedef struct
{
    DWORD           dwSize;
    DWORD           dwMask;                 // IN/OUT  Which Attributes to Get/Set
    SHELLVIEWID*    pvid;                   // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    // The folder's WebView template path
    LPWSTR          pszWebViewTemplate;     // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    DWORD           cchWebViewTemplate;     // IN - Specifies the size of the buffer pointed to by pszWebViewTemplate
                                            // Ignored if dwReadWrite is FCS_READ
    LPWSTR           pszWebViewTemplateVersion;  // currently IN only
    // Infotip for the folder
    LPWSTR          pszInfoTip;             // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    DWORD           cchInfoTip;             // IN - Specifies the size of the buffer pointed to by pszInfoTip
                                            // Ignored if dwReadWrite is FCS_READ
    // CLSID that points to more info in the registry
    CLSID*          pclsid;                 // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    // Other flags for the folder. Takes FCS_FLAG_* values
    DWORD           dwFlags;                // OUT - if dwReadWrite is FCS_READ, IN - otherwise


    LPWSTR           pszIconFile;           // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    DWORD            cchIconFile;           // IN - Specifies the size of the buffer pointed to by pszIconFile
                                            // Ignored if dwReadWrite is FCS_READ

    int              iIconIndex;            // OUT - if dwReadWrite is FCS_READ, IN - otherwise

    LPWSTR           pszLogo;               // OUT - if dwReadWrite is FCS_READ, IN - otherwise
    DWORD            cchLogo;               // IN - Specifies the size of the buffer pointed to by pszIconFile
                                            // Ignored if dwReadWrite is FCS_READ
} SHFOLDERCUSTOMSETTINGS, *LPSHFOLDERCUSTOMSETTINGS;

#include <poppack.h>        /* Return to byte packing */

// Gets/Sets the Folder Custom Settings for pszPath based on dwReadWrite. dwReadWrite can be FCS_READ/FCS_WRITE/FCS_FORCEWRITE
SHSTDAPI SHGetSetFolderCustomSettings(_Inout_ LPSHFOLDERCUSTOMSETTINGS pfcs, _In_ PCWSTR pszPath, DWORD dwReadWrite);
#endif

WinShellDispatcherTask* WindowsPlatform::shellDispatcherTask = NULL;

void WindowsPlatform::initialize(int argc, char *argv[])
{

}

bool WindowsPlatform::enableTrayIcon(QString executable)
{
    HRESULT hr;
    ITrayNotify *m_ITrayNotify;
    ITrayNotifyNew *m_ITrayNotifyNew;

    CoInitialize(NULL);
    hr = CoCreateInstance (
          __uuidof (TrayNotify),
          NULL,
          CLSCTX_LOCAL_SERVER,
          __uuidof (ITrayNotify),
          (PVOID *) &m_ITrayNotify);
    if (hr != S_OK)
    {
        hr = CoCreateInstance (
          __uuidof (TrayNotify),
          NULL,
          CLSCTX_LOCAL_SERVER,
          __uuidof (ITrayNotifyNew),
          (PVOID *) &m_ITrayNotifyNew);
        if (hr != S_OK)
        {
            return false;
        }
    }

    WinTrayReceiver *receiver = NULL;
    if (m_ITrayNotify)
    {
        receiver = new WinTrayReceiver(m_ITrayNotify, executable);
    }
    else
    {
        receiver = new WinTrayReceiver(m_ITrayNotifyNew, executable);
    }

    hr = receiver->start();
    if (hr != S_OK)
    {
        return false;
    }

    return true;
}

void WindowsPlatform::notifyItemChange(QString path)
{
    if (path.isEmpty())
    {
        return;
    }

    if (path.startsWith(QString::fromAscii("\\\\?\\")))
    {
        path = path.mid(4);
    }

    if (path.length() >= MAX_PATH)
    {
        return;
    }

    WCHAR *windowsPath = (WCHAR *)path.utf16();
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, windowsPath, NULL);
}

//From http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891.aspx
HRESULT WindowsPlatform::CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc,
                                 LPCWSTR pszIconfile, int iIconindex)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description.
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);
        if (pszIconfile)
        {
            psl->SetIconLocation(pszIconfile, iIconindex);
        }

        // Query IShellLink for the IPersistFile interface, used for saving the
        // shortcut in persistent storage.
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            // Save the link by calling IPersistFile::Save.
            hres = ppf->Save(lpszPathLink, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}

bool WindowsPlatform::startOnStartup(bool value)
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path);
    if (res != S_OK)
    {
        return false;
    }

    QString startupPath = QString::fromWCharArray(path);
    startupPath += QString::fromAscii("\\MEGAsync.lnk");

    if (value)
    {
        if (QFile(startupPath).exists())
        {
            return true;
        }

        WCHAR wDescription[]=L"Start MEGAsync";
        WCHAR *wStartupPath = (WCHAR *)startupPath.utf16();

        QString exec = MegaApplication::applicationFilePath();
        exec = QDir::toNativeSeparators(exec);
        WCHAR *wExecPath = (WCHAR *)exec.utf16();

        res = CreateLink(wExecPath, wStartupPath, wDescription);

        if (res != S_OK)
        {
            return false;
        }
        return true;
    }
    else
    {
        QFile::remove(startupPath);
        return true;
    }
}

bool WindowsPlatform::isStartOnStartupActive()
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path);
    if (res != S_OK)
    {
        return false;
    }

    QString startupPath = QString::fromWCharArray(path);
    startupPath += QString::fromAscii("\\MEGAsync.lnk");
    if (QFileInfo(startupPath).isSymLink())
    {
        return true;
    }
    return false;
}

void WindowsPlatform::showInFolder(QString pathIn)
{
    QString param;
    param = QLatin1String("/select,");
    param += QString::fromAscii("\"\"") + QDir::toNativeSeparators(pathIn) + QString::fromAscii("\"\"");
    QProcess::startDetached(QString::fromAscii("explorer ") + param);
}

void WindowsPlatform::startShellDispatcher(MegaApplication *receiver)
{
    if (shellDispatcherTask)
    {
        return;
    }
    shellDispatcherTask = new WinShellDispatcherTask(receiver);
    shellDispatcherTask->start();
}

void WindowsPlatform::stopShellDispatcher()
{
    if (shellDispatcherTask)
    {
        shellDispatcherTask->exitTask();
        shellDispatcherTask->wait();
        delete shellDispatcherTask;
        shellDispatcherTask = NULL;
    }
}

void WindowsPlatform::syncFolderAdded(QString syncPath, QString syncName)
{
    if (syncPath.startsWith(QString::fromAscii("\\\\?\\")))
    {
        syncPath = syncPath.mid(4);
    }

    if (!syncPath.size())
    {
        return;
    }

    QDir syncDir(syncPath);
    if (!syncDir.exists())
    {
        return;
    }

    DWORD dwVersion = GetVersion();
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    int iconIndex = (dwMajorVersion<6) ? 2 : 3;

    QString infoTip = QCoreApplication::translate("WindowsPlatform", "MEGA synced folder");
    SHFOLDERCUSTOMSETTINGS fcs = {0};
    fcs.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
    fcs.dwMask = FCSM_ICONFILE | FCSM_INFOTIP;
    fcs.pszIconFile = (LPWSTR)MegaApplication::applicationFilePath().utf16();
    fcs.iIconIndex = iconIndex;
    fcs.pszInfoTip = (LPWSTR)infoTip.utf16();
    SHGetSetFolderCustomSettings(&fcs, (LPCWSTR)syncPath.utf16(), FCS_FORCEWRITE);

    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if (res != S_OK)
    {
        return;
    }

    QString linksPath = QString::fromWCharArray(path);
    linksPath += QString::fromAscii("\\Links");
    QFileInfo info(linksPath);
    if (!info.isDir())
    {
        return;
    }

    QString linkPath = linksPath + QString::fromAscii("\\") + syncName + QString::fromAscii(".lnk");
    if (QFile(linkPath).exists())
    {
        return;
    }

    WCHAR wDescription[]=L"MEGAsync synchronized folder";
    linkPath = QDir::toNativeSeparators(linkPath);
    WCHAR *wLinkPath = (WCHAR *)linkPath.utf16();

    syncPath = QDir::toNativeSeparators(syncPath);
    WCHAR *wSyncPath = (WCHAR *)syncPath.utf16();

    QString exec = MegaApplication::applicationFilePath();
    exec = QDir::toNativeSeparators(exec);
    WCHAR *wExecPath = (WCHAR *)exec.utf16();
    res = CreateLink(wSyncPath, wLinkPath, wDescription, wExecPath);

    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, wLinkPath, NULL);

    WCHAR *wLinksPath = (WCHAR *)linksPath.utf16();
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, wLinksPath, NULL);
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, syncPath.utf16(), NULL);
}

void WindowsPlatform::syncFolderRemoved(QString syncPath, QString syncName)
{
    if (!syncPath.size())
    {
        return;
    }

    if (syncPath.startsWith(QString::fromAscii("\\\\?\\")))
    {
        syncPath = syncPath.mid(4);
    }

    SHFOLDERCUSTOMSETTINGS fcs = {0};
    fcs.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
    fcs.dwMask = FCSM_ICONFILE | FCSM_INFOTIP;
    SHGetSetFolderCustomSettings(&fcs, (LPCWSTR)syncPath.utf16(), FCS_FORCEWRITE);

    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if (res != S_OK)
    {
        return;
    }

    QString linksPath = QString::fromWCharArray(path);
    linksPath += QString::fromAscii("\\Links");
    QFileInfo info(linksPath);
    if (!info.isDir())
    {
        return;
    }

    QString linkPath = linksPath + QString::fromAscii("\\") + syncName + QString::fromAscii(".lnk");

    QFile::remove(linkPath);

    WCHAR *wLinkPath = (WCHAR *)linkPath.utf16();
    SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, wLinkPath, NULL);

    WCHAR *wLinksPath = (WCHAR *)linksPath.utf16();
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, wLinksPath, NULL);
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, syncPath.utf16(), NULL);
}

QByteArray WindowsPlatform::encrypt(QByteArray data, QByteArray key)
{
    DATA_BLOB dataIn;
    DATA_BLOB dataOut;
    DATA_BLOB entropy;

    dataIn.pbData = (BYTE *)data.constData();
    dataIn.cbData = data.size();
    entropy.pbData = (BYTE *)key.constData();
    entropy.cbData = key.size();

    if (!CryptProtectData(&dataIn, L"", &entropy, NULL, NULL, 0, &dataOut))
    {
        return data;
    }

    QByteArray result((const char *)dataOut.pbData, dataOut.cbData);
    LocalFree(dataOut.pbData);
    return result;
}

QByteArray WindowsPlatform::decrypt(QByteArray data, QByteArray key)
{
    DATA_BLOB dataIn;
    DATA_BLOB dataOut;
    DATA_BLOB entropy;

    dataIn.pbData = (BYTE *)data.constData();
    dataIn.cbData = data.size();
    entropy.pbData = (BYTE *)key.constData();
    entropy.cbData = key.size();

    if (!CryptUnprotectData(&dataIn, NULL, &entropy, NULL, NULL, 0, &dataOut))
        return data;

    QByteArray result((const char *)dataOut.pbData, dataOut.cbData);
    LocalFree(dataOut.pbData);
    return result;
}

QByteArray WindowsPlatform::getLocalStorageKey()
{
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        return QByteArray();
    }

    DWORD dwBufferSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);
    if (!dwBufferSize)
    {
        CloseHandle(hToken);
        return QByteArray();
    }

    PTOKEN_USER userToken = (PTOKEN_USER)new char[dwBufferSize];
    if (!GetTokenInformation(hToken, TokenUser, userToken, dwBufferSize, &dwBufferSize) ||
            !IsValidSid(userToken->User.Sid))
    {
        CloseHandle(hToken);
        delete userToken;
        return QByteArray();
    }

    DWORD dwLength =  GetLengthSid(userToken->User.Sid);
    QByteArray result((char *)userToken->User.Sid, dwLength);
    CloseHandle(hToken);
    delete userToken;
    return result;
}

QString WindowsPlatform::getDefaultOpenApp(QString extension)
{
    DWORD length = 0;
    ASSOCSTR type = ASSOCSTR_CONTENTTYPE;
    QString extensionWithDot = QString::fromUtf8(".") + extension;
    QString mimeType;

    HRESULT ret = AssocQueryString(0, type, (LPCWSTR)extensionWithDot.utf16(),
                                 NULL, NULL, &length);
    if (ret == S_FALSE)
    {
        WCHAR *buffer = new WCHAR[length];
        ret = AssocQueryString(0, type, (LPCWSTR)extensionWithDot.utf16(),
                               NULL, buffer, &length);
        if (ret == S_OK)
        {
            mimeType = QString::fromUtf16((ushort *)buffer);
        }
        delete [] buffer;
    }

    if (!mimeType.startsWith(QString::fromUtf8("video")))
    {
        return QString();
    }

    type = ASSOCSTR_EXECUTABLE;
    ret = AssocQueryString(0, type, (LPCWSTR)extensionWithDot.utf16(),
                                 NULL, NULL, &length);
    if (ret == S_FALSE)
    { 
        WCHAR *buffer = new WCHAR[length];
        ret = AssocQueryString(0, type, (LPCWSTR)extensionWithDot.utf16(),
                               NULL, buffer, &length);
        if (ret == S_OK)
        {
            QString result = QString::fromUtf16((ushort *)buffer);
            delete [] buffer;
            return result;
        }
        delete [] buffer;
    }

    WCHAR buff[MAX_PATH];
    if (SHGetFolderPath(0, CSIDL_PROGRAM_FILESX86, NULL, SHGFP_TYPE_CURRENT, buff) == S_OK)
    {
        QString path = QString::fromUtf16((ushort *)buff);
        path.append(QString::fromUtf8("\\Windows Media Player\\wmplayer.exe"));
        if (QFile(path).exists())
        {
            return path;
        }
    }
    return QString();
}

void WindowsPlatform::enableDialogBlur(QDialog *dialog)
{
    // this feature doesn't work well yet
    return;

    bool win10 = false;
    HWND hWnd = (HWND)dialog->winId();
    dialog->setAttribute(Qt::WA_TranslucentBackground, true);
    dialog->setAttribute(Qt::WA_NoSystemBackground, true);

    const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
    if (hModule)
    {
        struct ACCENTPOLICY
        {
            int nAccentState;
            int nFlags;
            int nColor;
            int nAnimationId;
        };
        struct WINCOMPATTRDATA
        {
            int nAttribute;
            PVOID pData;
            ULONG ulDataSize;
        };
        typedef BOOL(WINAPI*pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0xFFFFFFFF, 0 };
            WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hWnd, &data);
            win10 = true;
        }
        FreeLibrary(hModule);
    }

#if QT_VERSION >= 0x050200
    if (!win10)
    {
        QtWin::setCompositionEnabled(true);
        QtWin::extendFrameIntoClientArea(dialog, -1, -1, -1, -1);
        QtWin::enableBlurBehindWindow(dialog);
    }
#endif
}

void WindowsPlatform::activateBackgroundWindow(QDialog *window)
{
    DWORD currentThreadId = GetCurrentThreadId();
    DWORD foregroundThreadId;
    HWND foregroundWindow;
    bool threadAttached = false;

    if (QGuiApplication::applicationState() != Qt::ApplicationActive
        && (foregroundWindow = GetForegroundWindow())
        && (foregroundThreadId = GetWindowThreadProcessId(foregroundWindow, NULL))
        && (foregroundThreadId != currentThreadId))
    {
        threadAttached = AttachThreadInput(foregroundThreadId, currentThreadId, TRUE);
    }
    window->showMinimized();
    window->showNormal();
    if (threadAttached)
    {
        AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
    }
}
