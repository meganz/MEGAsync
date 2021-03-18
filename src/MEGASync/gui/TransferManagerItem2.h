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
        void updateUi(QExplicitlySharedDataPointer<TransferData> data, const int row);
        void forwardMouseEvent(QMouseEvent *me);

    signals:
        void cancelClearTransfers(int firstRow, int amount);
        void retryTransfer(TransferTag tag);

    private slots:
        void on_tPauseResumeTransfer_clicked();
        void on_tCancelClearTransfer_clicked();
        void on_tItemRetry_clicked();
        void onPauseStateChanged();

    private:
        Ui::TransferManagerItem *mUi;
        Preferences* mPreferences;
        mega::MegaApi* mMegaApi;
        TransferTag mTransferTag;

        bool mIsPaused;
        bool mIsFinished;
        bool mAreDlPaused;
        bool mAreUlPaused;
        int mRow;
};

#endif // TRANSFERMANAGERITEM2_H
