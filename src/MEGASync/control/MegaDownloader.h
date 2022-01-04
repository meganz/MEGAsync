#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QMap>
#include "megaapi.h"
#include "control/Utilities.h"

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    // If you want to manage public transfers in a different MegaApi object,
    // provide megaApiGuest
    MegaDownloader(mega::MegaApi *megaApi);
    virtual ~MegaDownloader();
    bool processDownloadQueue(QQueue<WrappedNode *> *downloadQueue, QString path, unsigned long long appDataId);
    void download(WrappedNode *parent, QString path, QString appData);

protected:
    void download(WrappedNode *parent, QFileInfo info, QString appData);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void finishedTransfers(unsigned long long appDataId);

private:
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);
};

#endif // MEGADOWNLOADER_H
