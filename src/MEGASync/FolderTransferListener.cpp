#include "FolderTransferListener.h"

#include "Utilities.h"

FolderTransferListener::FolderTransferListener(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FolderTransferUpdateEvent>("FolderTransferUpdateEvent");
}

void FolderTransferListener::onFolderTransferUpdate(mega::MegaApi *, mega::MegaTransfer *transfer, int stage,
                                                    uint32_t foldercount, uint32_t createdfoldercount, uint32_t filecount,
                                                    const char *currentFolder, const char *currentFileLeafname)

{
    if(!transfer->isSyncTransfer() && !transfer->isBackupTransfer())
    {
        FolderTransferUpdateEvent event;
        event.transferName = Utilities::getNodePath(transfer);
        event.stage = stage;
        event.foldercount = foldercount;
        event.createdfoldercount = createdfoldercount;
        event.filecount = filecount;
        event.currentFolder = QString::fromUtf8(currentFolder);
        event.currentFileLeafname = QString::fromUtf8(currentFileLeafname);

        emit folderTransferUpdated(event);
    }
}
