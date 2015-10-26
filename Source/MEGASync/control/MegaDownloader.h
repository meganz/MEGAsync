#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QMap>
#include "megaapi.h"

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    // If you want to manage public transfers in a different MegaApi object,
    // provide megaApiGuest
    MegaDownloader(mega::MegaApi *megaApi, mega::MegaApi *megaApiGuest = NULL);
    virtual ~MegaDownloader();
    void processDownloadQueue(QQueue<mega::MegaNode *> *downloadQueue, QString path);
    void download(mega::MegaNode *parent, QString path);

signals:
    void dupplicateDownload(QString localPath, QString name, mega::MegaHandle handle);

protected:
    void download(mega::MegaNode *parent, QFileInfo info);

    mega::MegaApi *megaApi;
    mega::MegaApi *megaApiGuest;
    QMap<mega::MegaHandle, QString> pathMap;
};

#endif // MEGADOWNLOADER_H
