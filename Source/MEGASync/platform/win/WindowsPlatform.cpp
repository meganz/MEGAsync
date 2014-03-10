#include "WindowsPlatform.h"
#include <Shlobj.h>

WinShellDispatcherTask* WindowsPlatform::shellDispatcherTask = NULL;

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
    if(hr != S_OK)
    {
        hr = CoCreateInstance (
          __uuidof (TrayNotify),
          NULL,
          CLSCTX_LOCAL_SERVER,
          __uuidof (ITrayNotifyNew),
          (PVOID *) &m_ITrayNotifyNew);
        if(hr != S_OK) return false;
    }

    WinTrayReceiver *receiver = NULL;
    if(m_ITrayNotify) receiver = new WinTrayReceiver(m_ITrayNotify, executable);
    else receiver = new WinTrayReceiver(m_ITrayNotifyNew, executable);

    hr = receiver->start();
    if(hr != S_OK) return false;

    return true;
}

void WindowsPlatform::notifyItemChange(QString path)
{
    if(path.startsWith(QString::fromAscii("\\\\?\\"))) path = path.mid(4);
    if(path.length()>=MAX_PATH) return;
    WCHAR *windowsPath = (WCHAR *)path.utf16();
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, windowsPath, NULL);
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
        if(pszIconfile)
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
    if(res != S_OK) return false;

    QString startupPath = QString::fromWCharArray(path);
    startupPath += QString::fromAscii("\\MEGAsync.lnk");

    if(value)
    {
        if(QFile(startupPath).exists()) return true;

        WCHAR wDescription[]=L"Start MEGAsync";
        WCHAR *wStartupPath = (WCHAR *)startupPath.utf16();

        QString exec = MegaApplication::applicationFilePath();
		exec = QDir::toNativeSeparators(exec);
        WCHAR *wExecPath = (WCHAR *)exec.utf16();

        res = CreateLink(wExecPath, wStartupPath, wDescription);

        if(res != S_OK) return false;
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
    if(res != S_OK)
        return false;

    QString startupPath = QString::fromWCharArray(path);
    startupPath += QString::fromAscii("\\MEGAsync.lnk");
    if(QFileInfo(startupPath).isSymLink()) return true;
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
    if(shellDispatcherTask) return;
    shellDispatcherTask = new WinShellDispatcherTask(receiver);
    shellDispatcherTask->start();
}

void WindowsPlatform::stopShellDispatcher()
{
    if(shellDispatcherTask)
    {
        shellDispatcherTask->exitTask();
        shellDispatcherTask = NULL;
    }
}

void WindowsPlatform::syncFolderAdded(QString syncPath, QString syncName)
{
    if(syncPath.startsWith(QString::fromAscii("\\\\?\\")))
        syncPath = syncPath.mid(4);

    QDir syncDir(syncPath);
    if(!syncDir.exists()) return;

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
    SHGetSetFolderCustomSettings(&fcs, syncPath.utf16(), FCS_FORCEWRITE);

    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if(res != S_OK) return;

    QString linksPath = QString::fromWCharArray(path);
    linksPath += QString::fromAscii("\\Links");
    QFileInfo info(linksPath);
    if(!info.isDir()) return;

    QString linkPath = linksPath + QString::fromAscii("\\") + syncName + QString::fromAscii(".lnk");
    if(QFile(linkPath).exists()) return;

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
    if(!syncPath.size()) return;

    if(syncPath.startsWith(QString::fromAscii("\\\\?\\")))
        syncPath = syncPath.mid(4);

    SHFOLDERCUSTOMSETTINGS fcs = {0};
    fcs.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
    fcs.dwMask = FCSM_ICONFILE | FCSM_INFOTIP;
    SHGetSetFolderCustomSettings(&fcs, syncPath.utf16(), FCS_FORCEWRITE);

    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if(res != S_OK) return;

    QString linksPath = QString::fromWCharArray(path);
    linksPath += QString::fromAscii("\\Links");
    QFileInfo info(linksPath);
    if(!info.isDir()) return;

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

    if(!CryptProtectData(&dataIn, L"", &entropy, NULL, NULL, 0, &dataOut))
        return data;

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
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return QByteArray();

    DWORD dwBufferSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);
    if(!dwBufferSize)
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
