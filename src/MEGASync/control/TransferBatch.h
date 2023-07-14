#ifndef TRANSFERBATCH_H
#define TRANSFERBATCH_H

#include "megaapi.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>

class TransferBatch
{
public:
    TransferBatch(unsigned long long appDataId);
    ~TransferBatch() = default;

    bool isEmpty();

    void cancel();

    void onScanCompleted(unsigned long long appDataId);

    QString description();

    mega::MegaCancelToken* getCancelTokenPtr();

    std::shared_ptr<mega::MegaCancelToken> getCancelToken();

private:
    std::shared_ptr<mega::MegaCancelToken> mCancelToken;
    bool mHasFinished;
    unsigned long long mAppDataId;
};

class BlockingBatch
{
public:
    BlockingBatch() = default;

    ~BlockingBatch();

    void add(std::shared_ptr<TransferBatch> _batch);
    void removeBatch();

    void cancelTransfer();

    void onScanCompleted(unsigned long long appDataId);

    bool isBlockingStageFinished();

    void setAsUnblocked();

    bool hasCancelToken();
    bool isValid() const ;
    bool isCancelled() const;

    bool hasNodes() const;

    std::shared_ptr<mega::MegaCancelToken> getCancelToken();

    QString description();

private:

   void clearBatch();

   bool cancelled = false;
   std::shared_ptr<TransferBatch> mBatch;
};

#endif // MEGADOWNLOADER_H
