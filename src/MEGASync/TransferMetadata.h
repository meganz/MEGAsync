#ifndef TRANSFERMETADATA_H
#define TRANSFERMETADATA_H

#include <QString>

struct TransferMetaData
{
    TransferMetaData(int direction, int total = 0, int pending = 0, QString path = QString())
                    : totalTransfers(total), pendingTransfers(pending),
                      totalFiles(0), totalFolders(0),
                      transfersFileOK(0), transfersFolderOK(0),
                      transfersFailed(0), transfersCancelled(0),
                      transferDirection(direction), localPath(path) {}

    int totalTransfers;
    int pendingTransfers;
    int totalFiles;
    int totalFolders;
    int transfersFileOK;
    int transfersFolderOK;
    int transfersFailed;
    int transfersCancelled;
    int transferDirection;
    QString localPath;
};

#endif // TRANSFERMETADATA_H
