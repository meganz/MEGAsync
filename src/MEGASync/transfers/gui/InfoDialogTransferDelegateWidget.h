#ifndef INFODIALOGTRANSFERDELEGATEWIDGET_H
#define INFODIALOGTRANSFERDELEGATEWIDGET_H

#include "megaapi.h"
#include "TransferBaseDelegateWidget.h"
#include "TransferRemainingTime.h"

#include <QDateTime>
#include <QFileInfo>
#include <QMenu>
#include <QWidget>

namespace Ui {
class InfoDialogTransferDelegateWidget;
}

class InfoDialogTransferDelegateWidget : public TransferBaseDelegateWidget
{
    Q_OBJECT

    static const QRect FullRect;

public:
    explicit InfoDialogTransferDelegateWidget(QWidget *parent = 0);

    ~InfoDialogTransferDelegateWidget();

    ActionHoverType mouseHoverTransfer(bool isHover, const QPoint &pos) override;

    void finishTransfer();

    void updateTransferState() override;
    void setFileNameAndType() override;
    void setType() override;

    void setFileType(const QString& fileName);
    QString getTransferName();

    bool mouseHoverRetryingLabel(QPoint pos);

    void updateFinishedTime();
    void loadDefaultTransferIcon() {}
    void updateAnimation() {}

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void copyTransferLink();
    void openTransferFolder();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void reset() override;

private slots: 
    void on_lShowInFolder_clicked();
    void on_lActionTransfer_clicked();

private:
    Ui::InfoDialogTransferDelegateWidget *mUi;
    mega::MegaApi *mMegaApi;
    bool mIsHover;
    TransferRemainingTime mTransferRemainingTime;

    void updateFinishedIco(TransferData::TransferTypes transferType, bool error);
    void updateTransferActive(const QExplicitlySharedDataPointer<TransferData> data);
    void updateTransferCompletedOrFailed(const QExplicitlySharedDataPointer<TransferData> data);
    void updateTransferCompleting(const QExplicitlySharedDataPointer<TransferData> data);
    void updateTransferControlsOnHold(const QString& speedText);
};

#endif // INFODIALOGTRANSFERDELEGATEWIDGET_H
