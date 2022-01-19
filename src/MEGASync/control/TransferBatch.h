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
        return copy;
    }

    bool isEmpty()
    {
        return transferIds.empty();
    }

    void add(const QString& appId)
    {
        transferIds.push_back(appId);
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
        for (auto b : batches)
        {
            delete b;
        }
        batches.clear();
    }

    void add(TransferBatch* batch)
    {
        batches.push_back(batch);

        delete blockingBatch;
        blockingBatch = batch->createCollectionCopy();
    }

    void onFileScanCompleted(const QString& appId)
    {
        onFolderScanCompleted(appId);
    }

    void onFolderScanCompleted(const QString& appId)
    {
        if (blockingBatch)
        {
            blockingBatch->remove(appId);
        }
    }

    bool isBlockingStageFinished()
    {
        return (blockingBatch) ? blockingBatch->isEmpty() : true;
    }

    void setAsUnblocked()
    {
        delete blockingBatch;
        blockingBatch = nullptr;
    }

    void onTransferFinished(const QString& appId)
    {
        auto batchIt = findAndUpdateBatch(appId);
        if (batchIt != batches.end())
        {
            if ((*batchIt)->isEmpty())
            {
                delete *batchIt;
                batches.erase(batchIt);
            }
        }
    }

    std::vector<TransferBatch*> batches;
    TransferBatch* blockingBatch = nullptr;

private:
    std::vector<TransferBatch*>::iterator findAndUpdateBatch(const QString& appId)
    {
        auto batchIt = batches.begin();
        for (; batchIt != batches.end(); ++batchIt)
        {
            if ((*batchIt)->remove(appId))
            {
                return batchIt;
            }
        }
        return batches.end();
    }
};

#endif // MEGADOWNLOADER_H
