#include "FolderTransferListener.h"

#include "Utilities.h"


FolderTransferListener::FolderTransferListener()
    : QObject(nullptr)
{
    qRegisterMetaType<FolderTransferUpdateEvent>("FolderTransferUpdateEvent");

    mProcessTimer.start(300);
    connect(&mProcessTimer, &QTimer::timeout, this, &FolderTransferListener::processEvent);
}

void FolderTransferListener::onFolderTransferUpdate(mega::MegaApi *, mega::MegaTransfer *transfer, int stage,
                                                    uint32_t foldercount, uint32_t createdfoldercount, uint32_t filecount,
                                                    const char *, const char *)

{
    if(!transfer->isSyncTransfer() && !transfer->isBackupTransfer())
    {
        FolderTransferUpdateEvent event;
        event.stage = stage;

        event.foldercount = foldercount;
        event.createdfoldercount = createdfoldercount;
        event.filecount = filecount;

        if(stage >= mega::MegaTransfer::STAGE_TRANSFERRING_FILES)
        {
            event.appData = std::string(transfer->getAppData());
            event.transferName = Utilities::getNodePath(transfer);
            emit folderTransferUpdated(event);
        }
        else
        {
            QMutexLocker lock(&mLock);
            mEventsReceivedMap.insert(transfer->getTag(), event);
        }
    }
}

//You may reset the data every time the user process a queue of uploads/downloads
void FolderTransferListener::reset()
{
    QMutexLocker lock(&mLock);
    mEventsReceivedMap.clear();
    mEventsToProcessMap.clear();
}

void FolderTransferListener::processEvent()
{
    auto folderCount(0);
    auto fileCount(0);
    auto createFolderCount(0);

    mLock.lock();
    mEventsToProcessMap = mEventsReceivedMap;
    mLock.unlock();

    if(!mEventsToProcessMap.isEmpty())
    {
        auto eventToSend = mEventsToProcessMap.first();

        foreach(auto event, mEventsToProcessMap)
        {
            folderCount += event.foldercount;
            fileCount += event.filecount;
            createFolderCount += event.createdfoldercount;
        }

        eventToSend.foldercount = folderCount;
        eventToSend.filecount = fileCount;
        eventToSend.createdfoldercount = createFolderCount;

        emit folderTransferUpdated(eventToSend);
    }
}
