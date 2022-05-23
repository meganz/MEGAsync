#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include "control/Utilities.h"
#include "control/TransferBatch.h"
#include "TransferMetadata.h"

#include "megaapi.h"

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QMap>

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    // If you want to manage public transfers in a different MegaApi object,
    // provide megaApiGuest
    MegaDownloader(mega::MegaApi *megaApi);
    virtual ~MegaDownloader() = default;
    bool processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                              const QString &path, unsigned long long appDataId);

protected:
    bool download(WrappedNode *parent, QFileInfo info, QString appData, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void finishedTransfers(unsigned long long appDataId);
    void startingTransfers();

private:
    void startDownload(WrappedNode* parent, const QString &appData,
                       const QString &currentPathWithSep, mega::MegaCancelToken* cancelToken);
    void downloadForeignDir(mega::MegaNode *node, const QString &appData, const QString &currentPathWithSep);
    QString buildEscapedPath(const char* nodeName, QString currentPathWithSep);
    bool createDirIfNotPresent(QString path);
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);

    void update(TransferMetaData* dataToUpdate, QString& appData, mega::MegaNode* node, const QString& path);

    bool mNoTransferStarted = true;
};

#endif // MEGADOWNLOADER_H
