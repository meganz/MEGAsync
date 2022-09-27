#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <TransferBatch.h>
#include "Preferences.h"
#include "megaapi.h"

#include <QFutureWatcher>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>

class MegaUploader : public QObject
{
    Q_OBJECT

public:
    MegaUploader(mega::MegaApi *megaApi);
    virtual ~MegaUploader();

    void upload(QString path, const QString &nodeName, mega::MegaNode *parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch> &transferBatch);
    bool filesdiffer(QFileInfo &source, QFileInfo &destination);
    void uploadRecursivelyIntoASyncedLocation(QFileInfo srcPath, QString destPath, mega::MegaNode *parent, unsigned long long appDataID);

signals:
    void uploadRecursivelyIntoASyncedLocationFinished(const QString& filePath);
    void startingTransfers(bool canBeCancelled);

protected:
    void upload(QFileInfo info, const QString& nodeName, mega::MegaNode *parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch>& transferBatch);

private slots:
    void onUploadRecursivelyIntoASyncedLocationFinished();

private:
    void startUpload(const QString& localPath, const QString& nodeName, unsigned long long appDataID, mega::MegaNode* parent, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    QMap<QString, std::shared_ptr<QFutureWatcher<void>>> mUploadRecursivelyIntoASyncedLocationFuture;
};

#endif // MEGAUPLOADER_H
