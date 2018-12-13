#ifndef CUSTOMTRANSFERITEM_H
#define CUSTOMTRANSFERITEM_H

#include <QWidget>
#include <QFileInfo>
#include <QDateTime>
#include <QMenu>
#include "TransferItem.h"
#include "megaapi.h"

namespace Ui {
class CustomTransferItem;
}

class CustomTransferItem : public TransferItem
{
    Q_OBJECT

public:
    explicit CustomTransferItem(QWidget *parent = 0);

    ~CustomTransferItem();

    void setFileName(QString fileName);
    void setType(int type, bool isSyncTransfer = false);
    void setTransferState(int value);
    QString getTransferName();

    bool getLinkButtonClicked(QPoint pos);
    void mouseHoverTransfer(bool isHover);
    bool mouseHoverRetryingLabel(QPoint pos);

    void finishTransfer();
    void updateTransfer();
    void updateFinishedTime();
    void loadDefaultTransferIcon() {}
    void updateAnimation() {}
    bool cancelButtonClicked(QPoint pos) { return false;}
    void setStateLabel(QString labelState);

    bool eventFilter(QObject *, QEvent * ev);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

private:
    Ui::CustomTransferItem *ui;
    mega::MegaApi *megaApi;
    int remainingUploads, remainingDownloads;
    int totalUploads, totalDownloads;

protected:
    bool getLinkButtonEnabled;
};

#endif // CUSTOMTRANSFERITEM_H
