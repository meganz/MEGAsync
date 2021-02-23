#ifndef TRANSFERMANAGERITEM2_H
#define TRANSFERMANAGERITEM2_H

#include "TransferItem2.h"

#include <QDateTime>

namespace Ui {
class TransferManagerItem;
}

class TransferManagerItem2 : public QWidget
{
        Q_OBJECT

public:
    explicit TransferManagerItem2(QWidget *parent = 0);

    void updateUi(const TransferItem2& transferItem);

    void forwardMouseEvent(QMouseEvent *me);

    private slots:
    void on_tPauseTransfer_clicked();
    void on_tCancelTransfer_clicked();

    private:
        Ui::TransferManagerItem *mUi;
        mega::MegaApi* mMegaApi;
        TransferTag mTransferTag;
        bool mIsPaused;
};

#endif // TRANSFERMANAGERITEM2_H
