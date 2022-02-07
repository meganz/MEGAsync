#include "TransferBatch.h"

TransferBatch::TransferBatch()
{
    cancelToken = std::shared_ptr<mega::MegaCancelToken>(mega::MegaCancelToken::createInstance());
}

TransferBatch* TransferBatch::createCollectionCopy()
{
    auto copy = new TransferBatch();
    copy->cancelToken = cancelToken;
    copy->files = files;
    copy->folders = folders;
    return copy;
}

bool TransferBatch::isEmpty()
{
    return (files == 0 && folders == 0);
}

void TransferBatch::add(bool isDir)
{
    isDir ? ++folders : ++files;
}

void TransferBatch::cancel()
{
    cancelToken->cancel();
}

void TransferBatch::onFileScanCompleted()
{
    if (files > 0)
    {
        --files;
    }
}

void TransferBatch::onFolderScanCompleted()
{
    if (folders > 0)
    {
        --folders;
    }
}

void TransferBatch::onTransferFinished(bool isDir)
{
    isDir ? --folders : --files;
}

QString TransferBatch::description()
{
    return QString::fromLatin1("%2 files, %3 folders").arg(files).arg(folders);
}

mega::MegaCancelToken* TransferBatch::getCancelTokenPtr()
{
    return cancelToken.get();
}


BlockingBatch::~BlockingBatch()
{
    clearBatch();
}

void BlockingBatch::add(TransferBatch* _batch)
{
    batch = std::shared_ptr<TransferBatch>(_batch->createCollectionCopy());
}

void BlockingBatch::cancelTransfer()
{
    if (batch)
    {
        batch->cancel();
    }
}

void BlockingBatch::onFileScanCompleted()
{
    if (batch)
    {
        batch->onFileScanCompleted();
    }
}

void BlockingBatch::onFolderScanCompleted()
{
    if (batch)
    {
        batch->onFolderScanCompleted();
    }
}

bool BlockingBatch::isBlockingStageFinished()
{
    if (batch)
    {
        return batch->isEmpty();
    }
    return true;
}

void BlockingBatch::setAsUnblocked()
{
    clearBatch();
}

void BlockingBatch::onTransferFinished(bool isFolderTransfer)
{
    if (batch)
    {
        batch->onTransferFinished(isFolderTransfer);
        if (batch->isEmpty())
        {
            clearBatch();
        }
    }
}

QString BlockingBatch::description()
{
    if (batch)
    {
        return batch->description();
    }
    return QString::fromLatin1("nullptr");
}

void BlockingBatch::clearBatch()
{
   batch = nullptr;
}
