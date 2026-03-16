#include "MegaUploader.h"

#include <QtCore>
#include <QApplication>
#include <QPointer>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

#ifndef WIN32
#include <utime.h>
#endif

using namespace mega;
using namespace std;

MegaUploader::MegaUploader(MegaApi *megaApi, std::shared_ptr<FolderTransferListener> _listener)
    : mFolderTransferListener(_listener),
    mFolderTransferListenerDelegate(std::make_shared<QTMegaTransferListener>(megaApi, mFolderTransferListener.get()))
{
    this->megaApi = megaApi;
}

void MegaUploader::upload(QString path, const QString& nodeName, std::shared_ptr<MegaNode> parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch>& transferBatch)
{
    QFileInfo info(path);

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());
    QString msg = QString::fromLatin1("Starting upload : '%1' - '%2' - '%3'").arg(info.fileName(), currentPath).arg(appDataID);
    megaApi->log(MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
    startUpload(currentPath, nodeName, appDataID, parent.get(), transferBatch ? transferBatch->getCancelTokenPtr() : nullptr);

    emit startingTransfers();
}

void MegaUploader::startUpload(const QString& localPath, const QString &nodeName, unsigned long long appDataID, MegaNode* parent, MegaCancelToken* cancelToken)
{
    QByteArray localPathArray = localPath.toUtf8();

    QByteArray appData = appDataID > 0 ? (QString::number(appDataID) + QLatin1Char('*')).toUtf8() : QByteArray();
    MegaUploadOptions options;
    options.fileName = nodeName.toStdString();
    options.appData = appData.isEmpty() ? nullptr : appData.constData();
    options.pitagTrigger = ::mega::MegaApi::PITAG_TRIGGER_PICKER;

    megaApi->startUpload(localPathArray.constData(),
                         parent,
                         cancelToken,
                         &options,
                         mFolderTransferListenerDelegate.get());
}
