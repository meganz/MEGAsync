#ifndef TRANSFERLISTENER_H
#define TRANSFERLISTENER_H

#include "FolderTransferEvents.h"
#include "megaapi.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimer>

class FolderTransferListener : public QObject, public mega::MegaTransferListener
{
Q_OBJECT

public:
    FolderTransferListener();

    void onFolderTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer, int stage, uint32_t foldercount, uint32_t createdfoldercount, uint32_t filecount, const char*, const char*);
    void reset();

signals:
    void folderTransferUpdated(FolderTransferUpdateEvent);

private:
    void processEvent();

    QMap<int, FolderTransferUpdateEvent> mEventsReceivedMap;
    QMap<int, FolderTransferUpdateEvent> mEventsToProcessMap;
    QTimer mProcessTimer;
    QMutex mLock;
};

#endif // TRANSFERLISTENER_H
