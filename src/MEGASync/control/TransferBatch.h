#ifndef TRANSFERBATCH_H
#define TRANSFERBATCH_H


#include <QString>
#include <QVector>
#include "megaapi.h"
#include <memory>

class TransferBatch
{
public:
    TransferBatch()
    {
        cancelToken = std::shared_ptr<mega::MegaCancelToken>(mega::MegaCancelToken::createInstance());
    }

    ~TransferBatch()
    {
    }

    TransferBatch* createCollectionCopy()
    {
        auto copy = new TransferBatch();
        copy->transferIds = transferIds;
        copy->cancelToken = cancelToken;
        copy->files = files;
        copy->folders = folders;
        return copy;
    }

    bool isEmpty()
    {
        return (files == 0 && folders == 0);
    }

    void add(const QString& appId, bool isDir)
    {
        transferIds.push_back(appId);
        isDir ? ++folders : ++files;
    }

    bool remove(const QString& appId)
    {
        return transferIds.removeOne(appId);
    }

    QString description()
    {
        QString itemCount = QString::fromLatin1("%1 items").arg(transferIds.size());
        if (transferIds.size() > 4)
        {
            return itemCount;
        }

        QString desc = itemCount + QString::fromLatin1(" (");
        for (const auto& id : transferIds)
        {
            desc += id + QString::fromLatin1(", ");
        }
        desc += QString::fromLatin1(")");

        return desc;
    }

    int files = 0;
    int folders = 0;

    QVector<QString> transferIds;
    std::shared_ptr<mega::MegaCancelToken> cancelToken;
};

class TransferBatches
{
public:
    TransferBatches()
    {
    }

    ~TransferBatches()
    {
    }

    void add(TransferBatch* batch)
    {
        delete blockingBatch;
        blockingBatch = batch->createCollectionCopy();
    }

    void cancelTransfer()
    {
        if (blockingBatch)
        {
            blockingBatch->cancelToken->cancel();
        }
    }

    void onFileScanCompleted(const QString& appId)
    {
        if (blockingBatch && blockingBatch->files > 0)
        {
            blockingBatch->remove(appId);
            --blockingBatch->files;
        }
    }

    void onFolderScanCompleted(const QString& appId)
    {
        if (blockingBatch && blockingBatch->folders > 0)
        {
            blockingBatch->remove(appId);
            --blockingBatch->folders;
        }
    }

    bool isBlockingStageFinished()
    {
        if (blockingBatch)
        {
            return (blockingBatch->files == 0 && blockingBatch->folders == 0);
        }
        return true;
    }

    void setAsUnblocked()
    {
        delete blockingBatch;
        blockingBatch = nullptr;
    }

    void onTransferFinished(bool isFolderTransfer)
    {
        if (blockingBatch)
        {
            isFolderTransfer ? --blockingBatch->folders : --blockingBatch->files;
            if (blockingBatch->isEmpty())
            {
                delete blockingBatch;
                blockingBatch = nullptr;
            }
        }
    }

    TransferBatch* blockingBatch = nullptr;
};

#endif // MEGADOWNLOADER_H
