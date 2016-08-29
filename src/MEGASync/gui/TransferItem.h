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
    QString getFileName();
    void setTransferredBytes(long long totalTransferredBytes, bool cancellable);
    void setTransferType(int type);
    void setSpeed(long long transferSpeed);
    void setTotalSize(long long size);
    void setType(int type, bool isSyncTransfer = false);
    void setPriority(unsigned long long priority);
    unsigned long long getPriority();

    void finishTransfer();
    void updateTransfer();
    void pauseTransfer();
    void mouseHoverTransfer(bool isHover);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void mouseEventClicked(QPoint pos, bool rightClick = false);

    ~TransferItem();

    int getTransferState();
    void setTransferState(int value);

    int getTransferTag();
    void setTransferTag(int value);

    bool getRegular();
    void setRegular(bool value);

private:
    Ui::TransferItem *ui;

protected:
    QString fileName;
    int type;
    int transferState;
    int transferTag;
    QMovie *animation;
    long long transferSpeed;
    long long meanTransferSpeed;
    long long speedCounter;
    long long totalSize;
    long long totalTransferredBytes;
    bool regular;
    unsigned long long priority;
};

Q_DECLARE_METATYPE(TransferItem*)

#endif // TRANSFERITEM_H
