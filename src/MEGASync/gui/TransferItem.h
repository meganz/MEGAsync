#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include <QWidget>

class TransferItem : public QWidget
{
    Q_OBJECT
public:

    enum {
        ACTION_BUTTON = 0,
        SHOW_IN_FOLDER_BUTTON,
    };

    explicit TransferItem(QWidget *parent = 0);

    virtual void setFileName(QString fileName);
    virtual QString getFileName();
    virtual void setTransferredBytes(long long totalTransferredBytes, bool cancellable);
    virtual void setSpeed(long long transferSpeed, long long meanSpeed);
    virtual void setTotalSize(long long size);
    virtual void setFinishedTime(long long time);

    virtual void setType(int type, bool isSyncTransfer = false);
    virtual int getType();

    virtual void setPriority(unsigned long long priority);
    virtual unsigned long long getPriority();

    virtual int getTransferState();
    virtual void setTransferState(int value);
    virtual bool isTransferFinished();

    virtual int getTransferError();
    virtual void setTransferError(int error, long long value);

    virtual int getTransferTag();
    virtual void setTransferTag(int value);

    virtual bool getRegular();
    virtual void setRegular(bool value);

    virtual bool getIsLinkAvailable();
    virtual void setIsLinkAvailable(bool value);

    virtual int getNodeAccess();
    virtual void setNodeAccess(int value);

    virtual void updateTransfer() = 0;
    virtual void updateFinishedTime() = 0;

    virtual void loadDefaultTransferIcon() = 0;
    virtual void updateAnimation() = 0;

    virtual bool cancelButtonClicked(QPoint pos) = 0;
    virtual bool checkIsInsideButton(QPoint pos, int button) = 0;
    virtual bool mouseHoverRetryingLabel(QPoint pos) = 0;
    virtual void mouseHoverTransfer(bool isHover) = 0;
    virtual void setStateLabel(QString labelState) = 0;
    virtual QString getTransferName() = 0;

signals:
    void refreshTransfer(int tag);

protected:
    QString fileName;
    int type;
    int transferState;
    int transferTag;
    int transferError;
    long long transferErrorValue;
    long long transferSpeed;
    long long meanTransferSpeed;
    long long totalSize;
    long long totalTransferredBytes;
    bool regular;
    bool isLinkAvailable;
    int nodeAccess;
    unsigned long long priority;
    bool cancelButtonEnabled;
    bool isSyncTransfer;
    long long dsFinishedTime;

};

#endif // TRANSFERITEM_H
