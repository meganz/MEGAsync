#include "WindowsUtils.h"

TrayNotificationReceiver::TrayNotificationReceiver(ITrayNotify *m_ITrayNotify, QString &executable)
{
    this->m_ITrayNotify = m_ITrayNotify;
    this->m_ITrayNotifyNew = NULL;
    this->executable = executable;
    m_cRef = 0;
}

TrayNotificationReceiver::TrayNotificationReceiver(ITrayNotifyNew *m_ITrayNotifyNew, QString &executable)
{
    this->m_ITrayNotifyNew = m_ITrayNotifyNew;
    this->m_ITrayNotify = NULL;
    this->executable = executable;
    m_cRef = 0;
}

boolean TrayNotificationReceiver::start()
{
    if(m_ITrayNotify) return m_ITrayNotify->RegisterCallback (this);
    else if(m_ITrayNotifyNew) return m_ITrayNotifyNew->RegisterCallback (this, &id);
    return false;
}

void TrayNotificationReceiver::stop()
{
    if(m_ITrayNotify)
    {
        m_ITrayNotify->Release();
        m_ITrayNotify = NULL;
    }
    else if(m_ITrayNotifyNew)
    {
        m_ITrayNotifyNew->UnregisterCallback(id);
        m_ITrayNotifyNew->Release();
        m_ITrayNotifyNew = NULL;
    }
}

TrayNotificationReceiver::~TrayNotificationReceiver()
{
    stop();
}

HRESULT __stdcall TrayNotificationReceiver::QueryInterface(REFIID riid, PVOID *ppv)
{
    if (ppv == NULL) return E_POINTER;

    if (riid == __uuidof (INotificationCB))
        *ppv = (INotificationCB *) this;
    else if (riid == IID_IUnknown)
        *ppv = (IUnknown *) this;
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG __stdcall TrayNotificationReceiver::AddRef(VOID)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

ULONG __stdcall TrayNotificationReceiver::Release(VOID)
{
    ULONG ulRefCount = InterlockedDecrement(&m_cRef);
    if (m_cRef == 0)
        delete this;
    return ulRefCount;
}


HRESULT __stdcall TrayNotificationReceiver::Notify (ULONG Event,
                          NOTIFYITEM *NotifyItem)
{
    if((m_ITrayNotify == NULL) && (m_ITrayNotifyNew==NULL)) return S_OK;
    if(Event != 0)
    {
        stop();
        return S_OK;
    }

    QString name = QString::fromWCharArray(NotifyItem->pszExeName);
    if(name.contains(executable))
    {
        cout << "TRAYICON Found!!" << endl;
        NotifyItem->dwPreference = 2;
        if(m_ITrayNotify) m_ITrayNotify->SetPreference(NotifyItem);
        else if(m_ITrayNotifyNew) m_ITrayNotifyNew->SetPreference(NotifyItem);
    }

    return S_OK;
}

boolean WindowsUtils::enableIcon(QString &executable)
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

void WindowsUtils::notifyNewFolder(QString &path)
{
    SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyFolderContentsChange(QString &path)
{
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyFolderDeleted(QString &path)
{
    SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyNewFile(QString &path)
{
    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyFileDeleted(QString &path)
{
    SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyItemChange(QString &path)
{
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, path.toStdString().c_str(), NULL);
}

void WindowsUtils::notifyAttributeChange(QString &path)
{
    SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATH, path.toStdString().c_str(), NULL);
}

#include <QDebug>

//From http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891.aspx
HRESULT WindowsUtils::CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc)
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
    if(res != S_OK)
        return false;

    QString startupPath = QString::fromWCharArray(path);
    startupPath += "\\MegaSync.lnk";

    if(value)
    {
        WCHAR executablePath[MAX_PATH];
        WCHAR description[]=L"Start MegaSync";

        int len = startupPath.toWCharArray(path);
        path[len] = L'\0';

        QString exec = QCoreApplication::applicationFilePath();
        exec.replace('/','\\');
        len = exec.toWCharArray(executablePath);
        executablePath[len]= L'\0';

        res = CreateLink(executablePath, path, description);
        if(res != S_OK)
            return false;

        return true;
    }
    else
    {
        QFile::remove(startupPath);
        return true;
    }
}


WindowsUtils::WindowsUtils()
{
}
