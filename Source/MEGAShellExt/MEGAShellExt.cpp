#include "MEGAShellExt.h"

#define SYNCED_ICON_ID		0
#define NOT_SYNCED_ICON_ID	1

ShellOverlayBinder1::ShellOverlayBinder1(QObject *parent) : QObject(parent) {}

QAxAggregated *ShellOverlayBinder1::createAggregate()
{
	return new MEGAShellExt(SYNCED_ICON_ID, this);
}

ShellOverlayBinder2::ShellOverlayBinder2(QObject *parent) : QObject(parent) {}

QAxAggregated *ShellOverlayBinder2::createAggregate()
{
	return new MEGAShellExt(NOT_SYNCED_ICON_ID, this);
}

//Global mutex
QMutex *mutex = new QMutex();

MEGAShellExt::MEGAShellExt(int id, QObject *parent) : QObject(parent)
{
    this->id = id;
    clientSocket = new QLocalSocket(this);
    clientSocket->connectToServer("MegaLocalServer");
}

long MEGAShellExt::queryInterface(const QUuid &iid, void **iface)
{
    *iface = 0;
    if (iid == IID_IShellIconOverlayIdentifier)
        *iface = (IShellIconOverlayIdentifier *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP MEGAShellExt::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
    QString iconPath(QAxFactory::serverFilePath());
    if (iconPath.length() > cchMax)
        return S_FALSE;

    int len = iconPath.toWCharArray(pwszIconFile);
    pwszIconFile[len] = L'\0';

    *pIndex = id;
    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
    return S_OK;
}

//This function is not important
STDMETHODIMP MEGAShellExt::GetPriority(int *pPriority)
{
    *pPriority = 0;
    return S_OK;
}

STDMETHODIMP MEGAShellExt::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
    mutex->lock();

    if(!clientSocket->isValid())
    {
        clientSocket->reset();
        clientSocket->connectToServer("MegaLocalServer");
        mutex->unlock();
        return S_FALSE;
    }

    if(!clientSocket->isWritable())
    {
        if(!clientSocket->waitForConnected())
        {
            mutex->unlock();
            return S_FALSE;
        }
    }

    QDataStream stream(clientSocket);
    stream.setVersion(QDataStream::Qt_4_8);
    stream << QString::fromWCharArray(pwszPath);
    clientSocket->flush();
    if(!clientSocket->waitForReadyRead())
    {
        mutex->unlock();
        return S_FALSE;
    }
    QString message;
    stream >> message;
    mutex->unlock();

    if(message.toInt()==id)
        return S_OK;

    return S_FALSE;
}
