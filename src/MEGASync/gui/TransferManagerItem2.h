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

        void updateUi(const TransferItem2& transferItem, const int row);

        void forwardMouseEvent(QMouseEvent *me);

    signals:
        void clearTransfer(int row);

    private slots:
        void on_tPauseResumeTransfer_clicked();
        void on_tCancelClearTransfer_clicked();

    private:
        Ui::TransferManagerItem *mUi;
        mega::MegaApi* mMegaApi;
        TransferTag mTransferTag;
        bool mIsPaused;
        bool mIsFinished;
        int mRow;
};

#endif // TRANSFERMANAGERITEM2_H
