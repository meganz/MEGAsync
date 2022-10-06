#ifndef TRANSFERLISTENER_H
#define TRANSFERLISTENER_H

#include "FolderTransferEvents.h"
#include <QObject>
#include <megaapi.h>

class FolderTransferListener : public QObject, public mega::MegaTransferListener
{
Q_OBJECT

public:
    FolderTransferListener(QObject* parent);

    void onFolderTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer, int stage, uint32_t foldercount, uint32_t createdfoldercount, uint32_t filecount, const char* currentFolder, const char* currentFileLeafname) override;

signals:
    void folderTransferUpdated(FolderTransferUpdateEvent);
};

#endif // TRANSFERLISTENER_H
