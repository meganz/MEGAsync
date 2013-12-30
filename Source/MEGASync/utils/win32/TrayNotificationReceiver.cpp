#include "TrayNotificationReceiver.h"

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
        if(m_ITrayNotifyNew) m_ITrayNotifyNew->Release();
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
        NotifyItem->dwPreference = 2;
        if(m_ITrayNotify) m_ITrayNotify->SetPreference(NotifyItem);
        else if(m_ITrayNotifyNew) m_ITrayNotifyNew->SetPreference(NotifyItem);
    }

    return S_OK;
}
