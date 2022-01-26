#ifndef TRANSFERMANAGERDELEGATEWIDGET_H
#define TRANSFERMANAGERDELEGATEWIDGET_H

#include "TransferItem.h"
#include "TransferBaseDelegateWidget.h"


#include <QDateTime>

namespace Ui {
class TransferManagerDelegateWidget;
}

class TransferManagerDelegateWidget : public TransferBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit TransferManagerDelegateWidget(QWidget* parent = 0);
    ~TransferManagerDelegateWidget();

    void finishTransfer(const QExplicitlySharedDataPointer<TransferData> data);

    void updateTransferState() override;
    void setFileNameAndType() override;
    void setType() override;

    void setFileType(const QString& fileName);

    bool mouseHoverTransfer(bool isHover, const QPoint &pos) override;

signals:
    void cancelClearTransfer();
    void pauseResumeTransfer();
    void retryTransfer();

private slots:
    void on_tPauseResumeTransfer_clicked();
    void on_tCancelClearTransfer_clicked();
    void on_tItemRetry_clicked();

private:
    bool setCancelClearTransferIcon(const QString &name);
    bool setPauseResumeTransferIcon(const QString &name);

    Ui::TransferManagerDelegateWidget *mUi;
    QString mLastPauseResuemtTransferIconName;
};

#endif // TRANSFERMANAGERDELEGATEWIDGET_H
