#ifndef TRANSFERTRACK_H
#define TRANSFERTRACK_H

#include <TransferItem.h>

#include <QObject>

class TransferTrack: public QObject
{
    Q_OBJECT

public:
    explicit TransferTrack(const QString& id, TransferData::TransferType type);

    void addTransferToTrack(const QVariant& transferId);
    void checkTransfer(QExplicitlySharedDataPointer<TransferData> data);

    bool areAllTransfersFinished() const;

    QString id() const;

signals:
    void transferStarted(TransferItem transfer);
    void transferFinished(TransferItem transfer);

private:
    QString mId;
    TransferData::TransferType mType;
    QVariantList mTrackedTransfers;
};

#endif // TRANSFERTRACK_H
