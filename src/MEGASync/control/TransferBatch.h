#ifndef TRANSFERBATCH_H
#define TRANSFERBATCH_H


#include <QString>
#include <QStringList>
#include <QVector>
#include "megaapi.h"
#include <memory>

class TransferBatch
{
public:
    TransferBatch();
    ~TransferBatch() = default;

    bool isEmpty();

    void add(const QString& nodePath);

    void cancel();

    void onScanCompleted(const QString& nodePath);

    QString description();

    mega::MegaCancelToken* getCancelTokenPtr();

    std::shared_ptr<mega::MegaCancelToken> getCancelToken();

private:
    QStringList mPendingNodes;
    std::shared_ptr<mega::MegaCancelToken> mCancelToken;
};

class BlockingBatch
{
public:
    BlockingBatch() = default;

    ~BlockingBatch();

    void add(std::shared_ptr<TransferBatch> _batch);
    void removeBatch();

    void cancelTransfer();

    void onScanCompleted(const QString& nodePath);

    bool isBlockingStageFinished();

    void setAsUnblocked();

    void onTransferFinished(const QString& nodePath);

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
