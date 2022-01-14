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
    virtual ~MegaDownloader() = default;
    bool processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, std::vector<WrappedNode*>* ongoingDownloads, QString path, unsigned long long appDataId);
    void download(WrappedNode *parent, QString path, QString appData);

protected:
    void download(WrappedNode *parent, QFileInfo info, QString appData);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void finishedTransfers(unsigned long long appDataId);
    void startingTransfers();

private:
    void startDownload(WrappedNode* parent, QString appData, QString currentPathWithSep);
    void downloadForeignDir(mega::MegaNode *node, QString appData, QString currentPathWithSep);
    QString buildEscapedPath(const char* nodeName, QString currentPathWithSep);
    bool createDirIfNotPresent(QString path);
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);

    bool noTransferStarted = true;
};

#endif // MEGADOWNLOADER_H
