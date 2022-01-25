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
        isDir ? ++folders : ++files;
    }

    int files = 0;
    int folders = 0;

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
            --blockingBatch->files;
        }
    }

    void onFolderScanCompleted(const QString& appId)
    {
        if (blockingBatch && blockingBatch->folders > 0)
        {
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
