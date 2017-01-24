#ifndef ACTIVETRANSFERSWIDGET_H
#define ACTIVETRANSFERSWIDGET_H

#include <QWidget>
#include <QMovie>
#include "QTMegaTransferListener.h"

namespace Ui {
class ActiveTransfersWidget;
}

struct TransferData
{
    QString fileName;
    int transferState;
    int tag;
    int type;
    long long transferSpeed;
    long long meanTransferSpeed;
    long long totalSize;
    long long totalTransferredBytes;
    unsigned long long priority;

    TransferData();
    void clear();
};

class ActiveTransfersWidget : public QWidget, public mega::MegaTransferListener
{
    Q_OBJECT

public:

    explicit ActiveTransfersWidget(QWidget *parent = 0);
    void init(mega::MegaApi *megaApi, mega::MegaTransfer *activeUpload = NULL, mega::MegaTransfer *activeDownload = NULL);
    ~ActiveTransfersWidget();

    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);

    void updateTransferInfo(mega::MegaTransfer *transfer);
    void pausedUpTransfers(bool paused);
    void pausedDownTransfers(bool paused);
    bool areTransfersActive();

public slots:
    void updateDownSpeed(long long speed);
    void updateUpSpeed(long long speed);

private slots:
    void on_bDownCancel_clicked();
    void on_bUpCancel_clicked();

private:
    Ui::ActiveTransfersWidget *ui;
    mega::MegaApi *megaApi;
    int totalUploads, totalDownloads;
    TransferData activeUpload, activeDownload;
    QMovie *animationDown, *animationUp;
    QPixmap loadIconResourceDown, loadIconResourceUp;

    void setType(TransferData *td, int type, bool isSyncTransfer);
    void setTotalSize(TransferData *td, long long size);
    void setSpeed(TransferData *td, long long transferSpeed);
    void setTransferredBytes(TransferData *td, long long totalTransferredBytes);

    void udpateTransferState(TransferData *td);
    void updateNumberOfTransfers(mega::MegaApi *api);
    void updateAnimation(TransferData *td);

    void changeEvent(QEvent *event);
};

#endif // ACTIVETRANSFERSWIDGET_H
