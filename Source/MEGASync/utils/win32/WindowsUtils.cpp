#include "WindowsUtils.h"
#include "utils/Utils.h"

ShellDispatcherTask* WindowsUtils::shellDispatcherTask = NULL;
QThread WindowsUtils::shellDispatcherThread;

boolean WindowsUtils::enableTrayIcon(QString executable)
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
        cout << "ITrayNotify failed. Trying ITrayNotifyNew..." << endl;
        hr = CoCreateInstance (
          __uuidof (TrayNotify),
          NULL,
          CLSCTX_LOCAL_SERVER,
          __uuidof (ITrayNotifyNew),
          (PVOID *) &m_ITrayNotifyNew);
        if(hr != S_OK) return false;
    }

    TrayNotificationReceiver *receiver = NULL;
    if(m_ITrayNotify) receiver = new TrayNotificationReceiver(m_ITrayNotify, executable);
    else receiver = new TrayNotificationReceiver(m_ITrayNotifyNew, executable);

    hr = receiver->start();
    if(hr != S_OK) return false;

    return true;
}

void WindowsUtils::notifyItemChange(QString path)
{
    WCHAR *windowsPath = new wchar_t[path.length()+1];
    int len = path.toWCharArray(windowsPath);
	windowsPath[len]=L'\0';
	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, windowsPath, NULL);
    delete windowsPath;
}

//From http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891.aspx
HRESULT WindowsUtils::CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc,
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

bool WindowsUtils::startOnStartup(bool value)
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path);
    if(res != S_OK) return false;

    QString startupPath = QString::fromWCharArray(path);
    startupPath += "\\MEGAsync.lnk";

    if(value)
    {
        WCHAR wDescription[]=L"Start MEGAsync";
        WCHAR *wStartupPath = new wchar_t[startupPath.length()+1];
        int len = startupPath.toWCharArray(wStartupPath);
        wStartupPath[len] = L'\0';

        QString exec = QCoreApplication::applicationFilePath();
		exec = QDir::toNativeSeparators(exec);
        WCHAR *wExecPath = new wchar_t[exec.length()+1];
        len = exec.toWCharArray(wExecPath);
        wExecPath[len]= L'\0';

        res = CreateLink(wExecPath, wStartupPath, wDescription);
        delete wStartupPath;
        delete wExecPath;

        if(res != S_OK) return false;
        return true;
    }
    else
    {
        QFile::remove(startupPath);
        return true;
    }
}

bool WindowsUtils::isStartOnStartupActive()
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path);
    if(res != S_OK)
        return false;

    QString startupPath = QString::fromWCharArray(path);
    startupPath += "\\MEGAsync.lnk";
    if(QFileInfo(startupPath).isSymLink()) return true;
    return false;
}

void WindowsUtils::showInFolder(QString pathIn)
{
    QString param;
    param = QLatin1String("/select,");
    param += "\"\"" + QDir::toNativeSeparators(pathIn) + "\"\"";
    QProcess::startDetached("explorer " + param);
}

void WindowsUtils::startShellDispatcher(MegaApplication *receiver)
{
    shellDispatcherTask = new ShellDispatcherTask(receiver);
    shellDispatcherTask->moveToThread(&shellDispatcherThread);
    shellDispatcherThread.start();
    QMetaObject::invokeMethod(shellDispatcherTask, "doWork", Qt::QueuedConnection);
}

void WindowsUtils::stopShellDispatcher()
{
    shellDispatcherTask->exitTask();
    shellDispatcherThread.quit();
    shellDispatcherThread.wait();
    delete shellDispatcherTask;
}

void WindowsUtils::syncFolderAdded(QString syncPath)
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if(res != S_OK) return;

    QString linksPath = QString::fromWCharArray(path);
    linksPath += "\\Links";
    QFileInfo info(linksPath);
    if(!info.isDir()) return;

    QFileInfo syncPathInfo(syncPath);
    QString linkPath = linksPath + "\\" + syncPathInfo.fileName() + ".lnk";

    WCHAR wDescription[]=L"MEGAsync synchronized folder";
    linkPath = QDir::toNativeSeparators(linkPath);
    WCHAR *wLinkPath = new wchar_t[linkPath.length()+1];
    int linkPathLen = linkPath.toWCharArray(wLinkPath);
    wLinkPath[linkPathLen] = L'\0';

    syncPath = QDir::toNativeSeparators(syncPath);
    WCHAR *wSyncPath = new wchar_t[syncPath.length()+1];
    int syncPathLen = syncPath.toWCharArray(wSyncPath);
    wSyncPath[syncPathLen] = L'\0';

    QString exec = QCoreApplication::applicationFilePath();
    exec = QDir::toNativeSeparators(exec);
    WCHAR *wExecPath = new wchar_t[exec.length()+1];
    int execPathLen = exec.toWCharArray(wExecPath);
    wExecPath[execPathLen] = L'\0';


    res = CreateLink(wSyncPath, wLinkPath, wDescription, wExecPath);
    delete wSyncPath;
    delete wExecPath;

    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, wLinkPath, NULL);
    delete wLinkPath;

    WCHAR *wLinksPath = new wchar_t[linksPath.length()+1];
    int linksPathLen = linksPath.toWCharArray(wLinksPath);
    wLinksPath[linksPathLen]=L'\0';
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, wLinksPath, NULL);
    delete wLinksPath;
}

void WindowsUtils::syncFolderRemoved(QString syncPath)
{
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
    if(res != S_OK) return;

    QString linksPath = QString::fromWCharArray(path);
    linksPath += "\\Links";
    QFileInfo info(linksPath);
    if(!info.isDir()) return;

    QFileInfo syncPathInfo(syncPath);
    QString linkPath = linksPath + "\\" + syncPathInfo.fileName() + ".lnk";

    QFile::remove(linkPath);

    WCHAR *wLinkPath = new wchar_t[linkPath.length()+1];
    int linkPathLen = linkPath.toWCharArray(wLinkPath);
    wLinkPath[linkPathLen]=L'\0';
    SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, wLinkPath, NULL);
    delete wLinkPath;

    WCHAR *wLinksPath = new wchar_t[linksPath.length()+1];
    int linksPathLen = linksPath.toWCharArray(wLinksPath);
    wLinksPath[linksPathLen]=L'\0';
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, wLinksPath, NULL);
    delete wLinksPath;
}
