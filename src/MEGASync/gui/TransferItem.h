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
    QString getTransferName();
    void setTransferredBytes(long long totalTransferredBytes, bool cancellable);
    void setTransferType(int type);
    void setSpeed(long long transferSpeed, long long meanSpeed);
    void setTotalSize(long long size);
    void setFinishedTime(long long time);
    void setStateLabel(QString labelState);

    void setType(int type, bool isSyncTransfer = false);
    int getType();
    void setPriority(unsigned long long priority);
    unsigned long long getPriority();

    void finishTransfer();
    void updateTransfer();
    void updateAnimation();
    void updateFinishedTime();
    void mouseHoverTransfer(bool isHover);
    void loadDefaultTransferIcon();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    bool cancelButtonClicked(QPoint pos);

    ~TransferItem();

    int getTransferState();
    void setTransferState(int value);

    int getTransferTag();
    void setTransferTag(int value);

    bool getRegular();
    void setRegular(bool value);

    bool eventFilter(QObject *, QEvent * ev);

signals:
    void refreshTransfer(int tag);

private:
    Ui::TransferItem *ui;

private slots:
    void frameChanged(int);

protected:
    QString fileName;
    int type;
    int transferState;
    int transferTag;
    QMovie *animation;
    QPixmap loadIconResource;
    long long transferSpeed;
    long long meanTransferSpeed;
    long long totalSize;
    long long totalTransferredBytes;
    bool regular;
    unsigned long long priority;
    bool cancelButtonEnabled;
    bool isSyncTransfer;
    long long dsFinishedTime;

};

#endif // TRANSFERITEM_H
