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
        explicit TransferManagerItem2(QWidget* parent = 0);
        ~TransferManagerItem2();
        void updateUi(const QExplicitlySharedDataPointer<TransferData> data, int row);
        void forwardMouseEvent(QMouseEvent* me);

    signals:
        void cancelClearTransfer(int row);
        void pauseResumeTransfer(int row, bool pauseState);
        void retryTransfer(TransferTag tag);

    private slots:
        void on_tPauseResumeTransfer_clicked();
        void on_tCancelClearTransfer_clicked();
        void on_tItemRetry_clicked();
        void onPauseStateChanged();

    private:
        Ui::TransferManagerItem *mUi;
        Preferences* mPreferences;
        TransferTag mTransferTag;
        bool mIsPaused;
        bool mIsFinished;
        bool mAreDlPaused;
        bool mAreUlPaused;
        int  mRow;
};

#endif // TRANSFERMANAGERITEM2_H
