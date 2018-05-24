#ifndef QCUSTOMTRANSFERSMODEL_H
#define QCUSTOMTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <deque>
#include <megaapi.h>
#include "QTransfersModel.h"
#include "QTMegaTransferListener.h"

class QCustomTransfersModel : public QTransfersModel
{
    Q_OBJECT

public:

    enum MODEL_STATE{
      IDLE = 0x01,
      UPLOAD = 0x02,
      DOWNLOAD = 0x04
    };

    explicit QCustomTransfersModel(int type, QObject *parent = 0);

    void refreshTransfers();
    virtual mega::MegaTransfer *getTransferByTag(int tag);

    // MegaApi callbacks
    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);

private slots:
    void refreshTransferItem(int tag);

protected:
    void updateTransferInfo(mega::MegaTransfer *transfer);
    void replaceWithTransfer(mega::MegaTransfer *transfer);

public slots:
    void removeAllTransfers();
    void removeTransferByTag(int transferTag);
    void updateActiveTransfer(mega::MegaApi *api, mega::MegaTransfer *newtransfer);

private:
    unsigned char modelState;
    int activeUploadTag;
    int activeDownloadTag;
    transfer_it getInsertPosition(mega::MegaTransfer *transfer);
};

#endif // QCUSTOMTRANSFERSMODEL_H
