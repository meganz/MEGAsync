#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include "FolderTransferListener.h"
#include "Preferences.h"
#include "megaapi.h"
#include "QTMegaRequestListener.h"

class MegaUploader : public QObject
{
    Q_OBJECT

public:
    MegaUploader(mega::MegaApi *megaApi, std::shared_ptr<FolderTransferListener> _listener);
    virtual ~MegaUploader() = default;

    bool upload(QString path, const QString &nodeName, mega::MegaNode *parent, unsigned long long appDataID, mega::MegaCancelToken *cancelToken);
    bool filesdiffer(QFileInfo &source, QFileInfo &destination);
    bool uploadRecursivelyIntoASyncedLocation(QFileInfo srcPath, QString destPath, mega::MegaNode *parent, unsigned long long appDataID, mega::MegaCancelToken *cancelToken);

protected:
    bool upload(QFileInfo info, const QString& nodeName, mega::MegaNode *parent, unsigned long long appDataID, mega::MegaCancelToken *cancelToken);

private:
    void startUpload(const QString& localPath, const QString& nodeName, unsigned long long appDataID, mega::MegaNode* parent, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    std::shared_ptr<FolderTransferListener> listener;
};

#endif // MEGAUPLOADER_H
