#ifndef TRANSFERBATCH_H
#define TRANSFERBATCH_H


#include <QString>
#include <QVector>
#include "megaapi.h"
#include <memory>

class TransferBatch
{
public:
    TransferBatch();
    ~TransferBatch() = default;

    TransferBatch* createCollectionCopy();

    bool isEmpty();

    void add(bool isDir);

    void cancel();

    void onFileScanCompleted();

    void onFolderScanCompleted();

    void onTransferFinished(bool isDir);

    QString description();

    mega::MegaCancelToken* getCancelTokenPtr();

    std::shared_ptr<mega::MegaCancelToken> getCancelToken();

private:
    int files = 0;
    int folders = 0;
    std::shared_ptr<mega::MegaCancelToken> cancelToken;
};

class BlockingBatch
{
public:
    BlockingBatch() = default;

    ~BlockingBatch();

    void add(std::shared_ptr<TransferBatch> _batch);

    void cancelTransfer();

    void onFileScanCompleted();

    void onFolderScanCompleted();

    bool isBlockingStageFinished();

    void setAsUnblocked();

    void onTransferFinished(bool isFolderTransfer);

    bool hasCancelToken();

    std::shared_ptr<mega::MegaCancelToken> getCancelToken();

    QString description();

private:

   void clearBatch();

   std::shared_ptr<TransferBatch> batch;
};

#endif // MEGADOWNLOADER_H
