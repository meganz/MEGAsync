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

    void add(bool isDir)
    {
        isDir ? ++folders : ++files;
    }

    void cancel()
    {
        cancelToken->cancel();
    }

    int files = 0;
    int folders = 0;

    std::shared_ptr<mega::MegaCancelToken> cancelToken;
};

class BlockingBatch
{
public:
    BlockingBatch()
    {
    }

    ~BlockingBatch()
    {
        clearBlockingBatch();
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
            blockingBatch->cancel();
        }
    }

    void onFileScanCompleted()
    {
        if (blockingBatch && blockingBatch->files > 0)
        {
            --blockingBatch->files;
        }
    }

    void onFolderScanCompleted()
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
            return blockingBatch->isEmpty();
        }
        return true;
    }

    void setAsUnblocked()
    {
        clearBlockingBatch();
    }

    void onTransferFinished(bool isFolderTransfer)
    {
        if (blockingBatch)
        {
            isFolderTransfer ? --blockingBatch->folders : --blockingBatch->files;
            if (blockingBatch->isEmpty())
            {
                clearBlockingBatch();
            }
        }
    }

    TransferBatch* blockingBatch = nullptr;

private:

   void clearBlockingBatch()
   {
       delete blockingBatch;
       blockingBatch = nullptr;
   }
};

#endif // MEGADOWNLOADER_H
