#ifndef TRANSFERMANAGERDELEGATEWIDGET_H
#define TRANSFERMANAGERDELEGATEWIDGET_H

#include "TransferItem.h"
#include "TransferBaseDelegateWidget.h"


#include <QDateTime>
#include <QUrl>

namespace Ui {
class TransferManagerDelegateWidget;
}

class TransferManagerDelegateWidget : public TransferBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit TransferManagerDelegateWidget(QWidget* parent = 0);
    ~TransferManagerDelegateWidget();

    ActionHoverType mouseHoverTransfer(bool isHover, const QPoint &pos) override;

    void render(const QStyleOptionViewItem &option, QPainter *painter, const QRegion &sourceRegion) override;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void reset() override;

signals:
    void cancelClearTransfer(bool isClear);
    void pauseResumeTransfer();
    void openTransfer();
    void retryTransfer();

private slots:
    void on_tPauseResumeTransfer_clicked();
    void on_tCancelClearTransfer_clicked();
    void on_tItemRetry_clicked();

private:
    void updateTransferState() override;
    void setFileNameAndType() override;
    void setType() override;
    void setFileType(const QString& fileName);
    void adjustFileName();

    bool setCancelClearTransferIcon(const QString &name);
    bool setPauseResumeTransferIcon(const QString &name);

    Ui::TransferManagerDelegateWidget *mUi;
    QString mPauseResumeTransferDefaultIconName;
};

#endif // TRANSFERMANAGERDELEGATEWIDGET_H
