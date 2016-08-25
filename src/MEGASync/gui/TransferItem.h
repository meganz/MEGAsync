#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include <QWidget>
#include <QDateTime>
#include <QMovie>

namespace Ui {
class TransferItem;
}

class TransferItem : public QWidget
{
    Q_OBJECT

public:
    explicit TransferItem(QWidget *parent = 0);

    void setFileName(QString fileName);
    void setTransferredBytes(long long totalTransferredBytes, bool cancellable);
    void setTransferType(int type);
    void setSpeed(long long transferSpeed);
    void setTotalSize(long long size);
    void setType(int type, bool isSyncTransfer = false);
    void setPriority(float priority);
    float getPriority();

    void finishTransfer();
    void updateTransfer();
    void mouseHoverTransfer(bool isHover);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void mouseEventClicked(QPoint pos, bool rightClick = false);

    ~TransferItem();

private:
    Ui::TransferItem *ui;

protected:
    QString fileName;
    int type;
    QMovie *animation;
    long long transferSpeed;
    long long totalSize, totalTransferredBytes;
    unsigned long long effectiveSpeed, effectiveTransferSpeed, lastUpdate, elapsedTransferTime;
    bool regular;
    float priority;
};

Q_DECLARE_METATYPE(TransferItem*)

#endif // TRANSFERITEM_H
